#include "Agent.h"

using namespace v8;
using namespace std;

Local<Array> WrapPopulation(std::vector<Solution*> population){
  v8::Local<v8::Array> jsArray = Nan::New<v8::Array>(population.size());
  for (size_t i = 0; i < population.size(); i++) {
    Nan::Set(jsArray, i, CreateWrappedSolution(*population.at(i)));
  }
  return jsArray;
}

Nan::Persistent<v8::FunctionTemplate> Agent::constructor;

NAN_MODULE_INIT(Agent::Init) {
  v8::Local<v8::FunctionTemplate> ctor = Nan::New<v8::FunctionTemplate>(Agent::New);
  constructor.Reset(ctor);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(Nan::New("Agent").ToLocalChecked());

  // link getters and setter to the object property
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("maxMachineRedundancy").ToLocalChecked(), Agent::HandleGetters, Agent::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("globalBestSolutions").ToLocalChecked(), Agent::HandleGetters, Agent::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("personalBestSolutions").ToLocalChecked(), Agent::HandleGetters, Agent::HandleSetters);

  // link functions
  Nan::SetPrototypeMethod(ctor, "addGlobalSolution", AddGlobalSolution);
  Nan::SetPrototypeMethod(ctor, "createSolution", _CreateSolution);
  Nan::SetPrototypeMethod(ctor, "createAndReturnSolution", _CreateAndReturnSolution);
  Nan::SetPrototypeMethod(ctor, "createSolutions", _CreateSolutions);

  target->Set(Nan::New("Agent").ToLocalChecked(), ctor->GetFunction());
}

NAN_METHOD(Agent::New) {
  // throw an error if constructor is called without new keyword
  if(!info.IsConstructCall()) {
    return Nan::ThrowError(Nan::New("Agent::New - called without new keyword").ToLocalChecked());
  }

  if(info.Length() != 6) {
    return Nan::ThrowError(Nan::New("Agent::New - unexpected number of arguments").ToLocalChecked());
  }
  // #NIT Arrays currently have no explicit type check
  if (!Nan::New(Matrix::constructor)->HasInstance(info[2])) {
    return Nan::ThrowError(Nan::New("Agent::New - expected argument 3 to be instance of Matrix").ToLocalChecked());
  }
  if (!Nan::New(Matrix::constructor)->HasInstance(info[3])) {
    return Nan::ThrowError(Nan::New("Agent::New - expected argument 4 to be instance of Matrix").ToLocalChecked());
  }
  if (!Nan::New(Matrix::constructor)->HasInstance(info[4])) {
    return Nan::ThrowError(Nan::New("Agent::New - expected argument 5 to be instance of Matrix").ToLocalChecked());
  }

  Nan::Callback *callback = new Nan::Callback(Nan::To<Function>(info[5]).ToLocalChecked());

  // create a new instance and wrap our javascript instance
  Agent* self = new Agent();
  self->Wrap(info.Holder());

  self->newBestSolutionCallback = callback;

  // initialize it's values
  Local<Array> factoryJsArray = Local<Array>::Cast(info[0]);
  vector<Factory*> factoryArray;
   for (unsigned int i = 0; i < factoryJsArray->Length(); i++)
  {
    Handle<Value> val = factoryJsArray->Get(i);
    Factory * factoryVal = Nan::ObjectWrap::Unwrap<Factory>(val->ToObject());
    factoryArray.push_back(factoryVal);
  }
  self->factories = factoryArray;

  Local<Array> machineJsArray = Local<Array>::Cast(info[1]);
  vector<Machine*> machineArray;
   for (unsigned int i = 0; i < machineJsArray->Length(); i++)
  {
    Handle<Value> val = machineJsArray->Get(i);
    Machine * machineVal = Nan::ObjectWrap::Unwrap<Machine>(val->ToObject());
    if (machineVal->redundancy > self->maxMachineRedundancy){
      self->maxMachineRedundancy = machineVal->redundancy;
    }
    machineArray.push_back(machineVal);
  }
  self->machines = machineArray;

  self->flowMatrix = Nan::ObjectWrap::Unwrap<Matrix>(info[2]->ToObject());
  self->changeOverMatrix = Nan::ObjectWrap::Unwrap<Matrix>(info[3]->ToObject());
  self->distanceMatrix = Nan::ObjectWrap::Unwrap<Matrix>(info[4]->ToObject());
  
  // return the wrapped javascript instance
  info.GetReturnValue().Set(info.Holder());
}

NAN_METHOD(Agent::AddGlobalSolution) {
  // unwrap this Agent
  Agent * self = Nan::ObjectWrap::Unwrap<Agent>(info.This());

  if (!Nan::New(Solution::constructor)->HasInstance(info[0])) {
    return Nan::ThrowError(Nan::New("Agent::AddGlobalSolution - expected argument to be instance of Solution").ToLocalChecked());
  }

  Solution* inSol = Nan::ObjectWrap::Unwrap<Solution>(info[0]->ToObject());
  if (inSol->quality < 0){
    return Nan::ThrowError(Nan::New("Agent::AddGlobalSolution - Solution must have a quality assigned").ToLocalChecked());
  }
  if (inSol->GetLength() != self->machines.size()){
    return Nan::ThrowError(Nan::New("Agent::AddGlobalSolution - Solution must have correct length").ToLocalChecked());
  }

  #ifdef DEBUG_OUTPUT
  printf("\nrecv addGlobalSolution callback\n");
  printf("prev globalBestSolutions: \n");
  for (unsigned int k = 0; k < self->globalBestSolutions.size(); k++){
    printf("%i %s \n", k, self->globalBestSolutions.at(k)->ToString().c_str());
  }
  #endif // DEBUG_OUTPUT

  // copy the incoming solution (js shouldn't keep a direct reference)
  // Update Populations handles this now...
  bool added = self->UpdateGlobalPopulation(*inSol);

  #ifdef DEBUG_OUTPUT
  printf("updating global population with new solution %s \n", inSol->ToString().c_str());
  printf("new globalBestSolutions: \n");
  for (unsigned int k = 0; k < self->globalBestSolutions.size(); k++){
    printf("%i %s \n", k, self->globalBestSolutions.at(k)->ToString().c_str());
  }
  #endif // DEBUG_OUTPUT
  info.GetReturnValue().Set(added);
}

NAN_GETTER(Agent::HandleGetters) {
  Agent* self = Nan::ObjectWrap::Unwrap<Agent>(info.This());
  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "maxMachineRedundancy") {
    info.GetReturnValue().Set(self->maxMachineRedundancy);
  } else if (propertyName == "maxGlobalBest"){
    info.GetReturnValue().Set(self->maxGlobalBest);
  } else if (propertyName == "globalBestSolutions"){
    info.GetReturnValue().Set(WrapPopulation(self->globalBestSolutions));
  } else if (propertyName == "maxPersonalBest"){
    info.GetReturnValue().Set(self->maxPersonalBest);
  } else if (propertyName == "personalBestSolutions"){
    info.GetReturnValue().Set(WrapPopulation(self->personalBestSolutions));
  } else if (propertyName == "pBestPopulationWeight"){
    info.GetReturnValue().Set(self->maxPersonalBest);
  } else if (propertyName == "gBestPopulationWeight"){
    info.GetReturnValue().Set(self->maxPersonalBest);
  } else if (propertyName == "rndWeight"){
    info.GetReturnValue().Set(self->rndWeight);
  } else {
    info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_SETTER(Agent::HandleSetters) {
  Agent* self = Nan::ObjectWrap::Unwrap<Agent>(info.This());
  return Nan::ThrowError(Nan::New("Agent::Not implemented.").ToLocalChecked());
}

int Agent::GetNextValue(int level, std::vector<int> prevSelections){
  Machine * currentMachine = machines.at(currentMachineIndex);
  // #PERFORMANCE
  // check the capacity requirement after selecting randomly
  // remove from availableFactories if not enough space
  // and select randomly again
  vector<int> availableFactories;
  for (unsigned int i = 0; i < factories.size(); i++){
    if (factories.at(i)->GetUnusedCapacity() >= currentMachine->size
      && !vectorContains(prevSelections, i)){
      availableFactories.push_back(i);
    }
  }
  if(availableFactories.size() == 0){
    return -1;
  }

  int selected = -1;
  random_selector<> selector{};
  // try until a new index is selected
  while (selected < 0){
    int populationSelector = GetRndNumberFromRange(0, rndWeight + gBestPopulationWeight + pBestPopulationWeight);
    if (populationSelector < rndWeight){
      // Select a random Factory from the available ones
      selected = selector(availableFactories);
    } else {
      // #PERFORMANCE
      // Cache those which are not possible
      int possibleIndex = -1;
      if (populationSelector < rndWeight + gBestPopulationWeight && globalBestSolutions.size() >= 1){
        Solution * srcSol = selector(globalBestSolutions);
        possibleIndex = srcSol->permutation.at(currentMachineIndex).at(level);
      } else if (populationSelector < rndWeight + gBestPopulationWeight + pBestPopulationWeight && personalBestSolutions.size() >= 1){
        Solution * srcSol = selector(personalBestSolutions);
        possibleIndex = srcSol->permutation.at(currentMachineIndex).at(level);
      }
      // Check whether there is a possible index which was copied from a previous Solution
      if (possibleIndex >= 0){
        // Check whether the machine fits into that factory (get the factory at that index)
        if (factories.at(possibleIndex)->GetUnusedCapacity() >= machines.at(currentMachineIndex)->size){
          // Check that the (redundant) machine is not placed in the same factory
          if (!vectorContains(prevSelections, possibleIndex)){
            // select that index, it is feasible           
            selected = possibleIndex;
          }
        }
      }
    }
  }
  // Use the size of currenMachine in the chosen factory
  factories.at(selected)->UseCapacity(currentMachine->size);
  return selected;
}

int Agent::GetNextValue(){
  std::vector<int> empty;
  return GetNextValue(0, empty);
}

void Agent::ResetFactories(){
  vector<Factory*> newFactories;
  for (unsigned int i = 0; i < factories.size(); i++){
    factories.at(i)->ResetUsedCapacity();
  }
}

int Agent::GetFlowDistanceSum(std::vector<std::vector<int>> permutation){
  int flowDistanceSum = 0;
  for (unsigned int m_i = 0; m_i < machines.size(); m_i++){
    for (unsigned int m_k = 0; m_k < machines.size(); m_k++){
      int flow = flowMatrix->GetValue(m_i, m_k);
      int distance = distanceMatrix->GetValue(permutation[m_i].at(0), permutation[m_k].at(0));
      flowDistanceSum += flow * distance;
    }
  }
  return flowDistanceSum;
}

/**
 * returns the flowDistanceSum if the factories with the indices from
 * failedFactories are failed.
 * returns -1 if there is no alternative with the provided permutation
 * and failed factories.
 */
int Agent::GetAltFlowDistanceSum(std::vector<std::vector<int>> permutation, std::vector<int> failedFactories){
  int flowDistanceSum = 0;
  for (unsigned int m_i = 0; m_i < machines.size(); m_i++){
    int f_i = getFirstNotInList(permutation[m_i], failedFactories);
    if (f_i < 0){
      return -1;
    }
    for (unsigned int m_k = 0; m_k < machines.size(); m_k++){
      int f_k = getFirstNotInList(permutation[m_k], failedFactories);
      if (f_k < 0){
        return -1;
      }
      int flow = flowMatrix->GetValue(m_i, m_k);
      int distance = distanceMatrix->GetValue(f_i, f_k);
      flowDistanceSum += flow * distance;
    }
  }
  return flowDistanceSum;
}

double Agent::GetRelativeAltFlowDistance(
  int referenceFlowDistance,
  std::vector<std::vector<int>> permutation,
  std::vector<int> failedFactories
){
  int altFlowDistanceSum = GetAltFlowDistanceSum(permutation, failedFactories);
  if (altFlowDistanceSum < 0){
    return 1;
  }
  if (altFlowDistanceSum < referenceFlowDistance){
    return 0;
  }
  return (double)(1 - (double)referenceFlowDistance/(double)altFlowDistanceSum);
}

double Agent::GetSingleFactoryFailureScore(
  int referenceFlowDistance,
  std::vector<std::vector<int>> permutation
) {
  double score = 0;
  vector<int> failedFactories;
  failedFactories.push_back(0);
  for (unsigned int f_i = 0; f_i < factories.size(); f_i++){
    failedFactories[0] = f_i;
    double relAltFlowDistance = GetRelativeAltFlowDistance(referenceFlowDistance, permutation, failedFactories);
    #ifdef DEBUG_OUTPUT
      printf("Factory %i, pFailure %f, relAltFlowDistance %f", f_i, factories[f_i]->pFailure, relAltFlowDistance);
      printf(" score %f", score);
    #endif
    score += factories[f_i]->pFailure * relAltFlowDistance * referenceFlowDistance;
    #ifdef DEBUG_OUTPUT
      printf(" => %f\n", score);
    #endif
  }
  return score;
}

double Agent::GetFailureRiskSum(std::vector<std::vector<int>> permutation){
  /**
   * Start with 1, if the failureRisk is zero (usually QAP case) the resulting sum will be 1
   **/
  double failureRiskSum = 1;
  for (unsigned int i = 0; i < permutation.size(); i++){
    double machineFailure = 1;
    for (unsigned int k = 0; k < permutation[i].size(); k++){
      /**
       * permutation[i][k] may be -1 if there wasn't any factory available
       * to asssign. In that case the backup factory is not available (to
       * minimize the machineFailure risk).
       */
      if (permutation[i][k] >= 0){
        machineFailure = machineFailure * factories.at(permutation[i][k])->pFailure;
      }
    }
    failureRiskSum += machineFailure;
  }
  return failureRiskSum;
}

int Agent::RateSolution(Solution &sol){
  // Rate the generated solution
  // #TODO rate all criteria (currently only flow*distance)
  sol.flowDistanceSum = GetFlowDistanceSum(sol.permutation);
  #ifdef QAP_ONLY
    sol.quality = sol.flowDistanceSum;
  #else
    sol.failureRiskSum = GetFailureRiskSum(sol.permutation);
    sol.singleFactoryFailureScore = GetSingleFactoryFailureScore(sol.flowDistanceSum, sol.permutation);
    // aggregate the scores
    sol.quality = sol.flowDistanceSum * sol.failureRiskSum + sol.singleFactoryFailureScore;
  #endif // ONLY_FLOW_DISTANCE
  #ifdef DEBUG_OUTPUT
  printf("RatedSolution %s\n", sol.ToString().c_str());
  #endif // DEBUG_OUTPUT
  return sol.quality;
}

void Agent::Solve(Solution &sol){
  ResetFactories();
  #ifdef DEBUG_OUTPUT
  printf("\nsolve with globalBestSolutions: \n");
  for (unsigned int k = 0; k < globalBestSolutions.size(); k++){
    printf("%i %s \n", k, globalBestSolutions.at(k)->ToString().c_str());
  }
  printf("\nsolve with personalBestSolutions: \n");
  for (unsigned int k = 0; k < personalBestSolutions.size(); k++){
    printf("%i %s \n", k, personalBestSolutions.at(k)->ToString().c_str());
  }
  #endif // DEBUG_OUTPUT

  int rLevel = 0;
  // Creates the first permutation (which selects a factory for each machine)
  currentMachineIndex = 0;
  while(currentMachineIndex < machines.size()){
    sol.Set(currentMachineIndex, rLevel, GetNextValue());
    currentMachineIndex++;
  }
  /**
   * Creates the additional "permutations" to satisfy the redundancy requirement
   * for each machine. (For normal QAP instances this is not run since redundacy
   * and capacity are always 1 --> One Machine per Factory for QAP)
   */
  #ifndef QAP_ONLY
  rLevel++;
  while(rLevel < maxMachineRedundancy){
    currentMachineIndex = 0;
    while(currentMachineIndex < machines.size()){
      if (rLevel < machines[currentMachineIndex]->redundancy){
        int nextVal = GetNextValue(rLevel, sol.permutation[currentMachineIndex]);
        int prevVal = sol.permutation[currentMachineIndex][rLevel - 1];
        sol.Set(currentMachineIndex, rLevel, nextVal);
      }
      currentMachineIndex++;
    }
    rLevel++;
  }
  #endif // QAP_ONLY
  /**
   * The created solution might not justify the redundancy requirements if there
   * wasn't any capacity left to fulfil the requirements.
   */
}

/**
 * There shouldn't be any solutions in here which might be interfered with by js!
 * It seems to cause issues during longer runs. (garbage collection?)
 */
void PlaceSolAtPosition(std::vector<Solution*> &population, Solution* sol, int i){
  if (i < population.size()){
    population[i] = sol;
  } else {
    population.push_back(sol);
  }
}

bool UpdatePopulation(std::vector<Solution*> &population, int populationSize, Solution &inSol){
  /**
   * Iterate through the population starting with the worts solution in the population.
   * Usually the loop is exited after the first check, because the new solution is worse
   * than all solutions in the population.
   */
  int i = population.size() - 1;
  while (i >= 0){
    // smaller quality is better
    if ( inSol.quality < population[i]->quality){
      if (i + 1 < populationSize){
        PlaceSolAtPosition(population, population[i], i + 1);
      } else {
        /**
         * Solutions are copied into the population.
         * --> A Solution is only references from one populaiton, even
         * if the same (quality + permutaion) solution is part of
         * multiple populations. (a copy is made)
         * Therefore it is safe to free the memory here.
         */
        delete (population[i]);
      }
      i--;
    } else {
      break;
    }
  }
  if (i + 1 < populationSize){
    /**
     * To allow efficiently freeing memory of solutions which drop out of the population
     * a copy of the solution is added to the population.
     * - This ensures that different populations do not have references to the same solution.
     * - Also it is assured that solutions which did not make it into the population can be
     *   deleted
     */
    Solution* sol = new Solution(inSol);
    PlaceSolAtPosition(population, sol, i + 1);
    return true;
  }
  return false;
}

bool Agent::UpdatePersonalPopulation(Solution &sol){
  return UpdatePopulation(personalBestSolutions, maxPersonalBest, sol);
}

bool Agent::UpdateGlobalPopulation(Solution &sol){
  return UpdatePopulation(globalBestSolutions, maxGlobalBest, sol);
}

void Agent::ReportNewBestSolution(Solution &sol){
  // Create new wrapped solution instance _sol
  Local<Object> _sol = CreateWrappedSolution(sol);

  // Call Callback with that wrapped instance
  const int callback_argc = 2;
  Local<Value> callback_argv[] = {
        Nan::Null()
      , _sol
    };

  newBestSolutionCallback->Call(callback_argc, callback_argv);
}

bool Agent::HandleNewSolution(Solution &sol){
  const bool isInPersonalBest = UpdatePersonalPopulation(sol);
  if (isInPersonalBest){
      const bool isInLocalGlobalBest = UpdateGlobalPopulation(sol);
      if (isInLocalGlobalBest){
        ReportNewBestSolution(sol);
      }
  }
  return isInPersonalBest;
}

Solution* Agent::CreateSolution(){
  Solution* sol = new Solution();
  Solve(*sol);
  RateSolution(*sol);
  HandleNewSolution(*sol);
  return sol;
}

void Agent::CreateSolutions(int n){
  for (int i = 0; i < n; i++){
    Solution* sol = CreateSolution();
    delete sol;
  }
}

NAN_METHOD(Agent::_CreateSolution) {
  Agent * self = Nan::ObjectWrap::Unwrap<Agent>(info.This());
  Solution* sol = self->CreateSolution();
  delete sol;
}

NAN_METHOD(Agent::_CreateAndReturnSolution) {
  // unwrap this Agent
  Agent * self = Nan::ObjectWrap::Unwrap<Agent>(info.This());
  // create new solution (native)
  Solution* sol = self->CreateSolution();
  /**
   * return a wrapped copy to make sure that the create solution
   * which is stored in the population wont be affected by any
   * js stuff
   */
  Local<Object> _sol = CreateWrappedSolution(*sol);
  /**
   * Solutions are copied into the population if they make it.
   */
  delete sol;
  info.GetReturnValue().Set(_sol);
}

NAN_METHOD(Agent::_CreateSolutions) {
  Agent * self = Nan::ObjectWrap::Unwrap<Agent>(info.This());
  int n = 1;
  if(info[0]->IsNumber()) {
    n = info[0]->NumberValue();
  }
  self->CreateSolutions(n);
}
