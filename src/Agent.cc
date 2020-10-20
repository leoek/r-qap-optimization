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

Local<Array> WrapPopulation(std::deque<Solution*> population){
  v8::Local<v8::Array> jsArray = Nan::New<v8::Array>(population.size());
  for (size_t i = 0; i < population.size(); i++) {
    Nan::Set(jsArray, i, CreateWrappedSolution(*population.at(i)));
  }
  return jsArray;
}

void UnWrapPopulation(std::vector<Solution*> population, v8::Local<v8::Value> value){
  Local<Array> jsArray = Local<Array>::Cast(value);
    for (unsigned int i = 0; i < population.size(); i++){
      delete population[i];
    }
    population.clear();
    for (unsigned int i = 0; i < jsArray->Length(); i++){
      Handle<Value> val = jsArray->Get(i);
      Solution* sol = Nan::ObjectWrap::Unwrap<Solution>(val->ToObject());
      population.push_back(sol);
    }
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

  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("maxPersonalBest").ToLocalChecked(), Agent::HandleGetters, Agent::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("maxGlobalBest").ToLocalChecked(), Agent::HandleGetters, Agent::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("pBestPopulationWeight").ToLocalChecked(), Agent::HandleGetters, Agent::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("gBestPopulationWeight").ToLocalChecked(), Agent::HandleGetters, Agent::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("rndWeight").ToLocalChecked(), Agent::HandleGetters, Agent::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("pHistoryWeight").ToLocalChecked(), Agent::HandleGetters, Agent::HandleSetters);

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

  if(info.Length() < 6) {
    return Nan::ThrowError(Nan::New(
      "Agent::New - unexpected number of arguments, min these 6 need to be supplied: factoryArray, machineArray, flowMatrix, changeOverMatrix, distanceMatrix, newBestCallBack"
      ).ToLocalChecked());
  }
  if(info.Length() > 15) {
    return Nan::ThrowError(Nan::New(
      "Agent::New - too many arguments supply these: factoryArray, machineArray, flowMatrix, changeOverMatrix, distanceMatrix, newBestCallBack, maxPersonalBest, maxGlobalBest, maxPersonalHistory, pBestPopulationWeight, gBestPopulationWeight, rndWeight, pHistoryWeight"
      ).ToLocalChecked());
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

  // create a new instance and wrap our javascript instance
  Agent* self = new Agent();
  self->Wrap(info.Holder());

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

  Nan::Callback *callback = new Nan::Callback(Nan::To<Function>(info[5]).ToLocalChecked());
  self->newBestSolutionCallback = callback;

  if(info[6]->IsNumber()) {
    self->maxPersonalBest = info[6]->NumberValue();
  }
  if(info[7]->IsNumber()) {
    self->maxGlobalBest = info[7]->NumberValue();
  }
  if(info[8]->IsNumber()) {
    self->maxPersonalHistory = info[8]->NumberValue();
  }
  if(info[9]->IsNumber()) {
    self->maxIterationBest = info[9]->NumberValue();
  }
  if(info[10]->IsNumber()) {
    self->pBestPopulationWeight = info[10]->NumberValue();
  }
  if(info[11]->IsNumber()) {
    self->gBestPopulationWeight = info[11]->NumberValue();
  }
  if(info[12]->IsNumber()) {
    self->rndWeight = info[12]->NumberValue();
  }
  if(info[13]->IsNumber()) {
    self->pHistoryWeight = info[13]->NumberValue();
  }
  if(info[14]->IsNumber()) {
    self->iterationBestWeight = info[14]->NumberValue();
  }
  
  #ifdef DEBUG_OUTPUT
  printf("\nCreated New Agent:\n%s\n", self->ToString().c_str());
  #endif // DEBUG_OUTPUT

  self->generateQualityScoreReferences();

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

NAN_METHOD(Agent::AddIterationBestSolution) {
  // unwrap this Agent
  Agent * self = Nan::ObjectWrap::Unwrap<Agent>(info.This());
  #ifdef DEBUG_OUTPUT // disable these checks as they would run very often...
  if (!Nan::New(Solution::constructor)->HasInstance(info[0])) {
    return Nan::ThrowError(Nan::New("Agent::AddIterationBestSolution - expected argument to be instance of Solution").ToLocalChecked());
  }
  #endif // DEBUG_OUTPUT
  Solution* inSol = Nan::ObjectWrap::Unwrap<Solution>(info[0]->ToObject());
  #ifdef DEBUG_OUTPUT
  if (inSol->quality < 0){
    return Nan::ThrowError(Nan::New("Agent::AddGlobalSolution - Solution must have a quality assigned").ToLocalChecked());
  }
  if (inSol->GetLength() != self->machines.size()){
    return Nan::ThrowError(Nan::New("Agent::AddGlobalSolution - Solution must have correct length").ToLocalChecked());
  }
  #endif // DEBUG_OUTPUT
  bool added = self->UpdateIterationBestPopulation(*inSol);
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
  } else if (propertyName == "maxPersonalHistory"){
    info.GetReturnValue().Set(self->maxPersonalHistory);
  } else if (propertyName == "personalHistorySolutions"){
    info.GetReturnValue().Set(WrapPopulation(self->personalHistorySolutions));
  } else if (propertyName == "maxIterationBest"){
    info.GetReturnValue().Set(self->maxIterationBest);
  } else if (propertyName == "personalBestSolutions"){
    info.GetReturnValue().Set(WrapPopulation(self->iterationBestSolutions));
  } else if (propertyName == "pBestPopulationWeight"){
    info.GetReturnValue().Set(self->pBestPopulationWeight);
  } else if (propertyName == "gBestPopulationWeight"){
    info.GetReturnValue().Set(self->gBestPopulationWeight);
  } else if (propertyName == "rndWeight"){
    info.GetReturnValue().Set(self->rndWeight);
  } else if (propertyName == "pHistoryWeight"){
    info.GetReturnValue().Set(self->pHistoryWeight);
  } else if (propertyName == "iterationBestWeight"){
    info.GetReturnValue().Set(self->iterationBestWeight);
  } else {
    info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_SETTER(Agent::HandleSetters) {
  Agent* self = Nan::ObjectWrap::Unwrap<Agent>(info.This());
  
  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "pBestPopulationWeight"){
    if(!value->IsNumber()) {
      return Nan::ThrowError(Nan::New("expected pBestPopulationWeight to be a number").ToLocalChecked());
    }
    self->pBestPopulationWeight = value->NumberValue();
  } else if (propertyName == "gBestPopulationWeight"){
    if(!value->IsNumber()) {
      return Nan::ThrowError(Nan::New("expected gBestPopulationWeight to be a number").ToLocalChecked());
    }
    self->gBestPopulationWeight = value->NumberValue();
  } else if (propertyName == "rndWeight"){
    if(!value->IsNumber()) {
      return Nan::ThrowError(Nan::New("expected rndWeight to be a number").ToLocalChecked());
    }
    self->rndWeight = value->NumberValue();
  } else if (propertyName == "pHistoryWeight"){
    if(!value->IsNumber()) {
      return Nan::ThrowError(Nan::New("expected pHistoryWeight to be a number").ToLocalChecked());
    }
    self->pHistoryWeight = value->NumberValue();
  } else if (propertyName == "iterationBestWeight"){
    if(!value->IsNumber()) {
      return Nan::ThrowError(Nan::New("expected iterationBestWeight to be a number").ToLocalChecked());
    }
    self->iterationBestWeight = value->NumberValue();
  } else if (propertyName == "personalBestSolutions"){
    if(!value->IsArray()) {
      return Nan::ThrowError(Nan::New("expected personalBestSolutions to be an Array").ToLocalChecked());
    }
    UnWrapPopulation(self->personalBestSolutions, value);
  } else if (propertyName == "globalBestSolutions"){
    if(!value->IsArray()) {
      return Nan::ThrowError(Nan::New("expected globalBestSolutions to be an Array").ToLocalChecked());
    }
    UnWrapPopulation(self->personalBestSolutions, value);
  } else {
    return Nan::ThrowError(Nan::New("Agent::Not implemented.").ToLocalChecked());
  }
}

std::string Agent::ToString(){
  std::string result = string_format(
    "[Agent] #machines: %i; maxMachineRedundancy: %i; #factories %i;",
    machines.size(), maxMachineRedundancy, factories.size());
  result += string_format(
    " maxGlobalBest: %i; maxPersonalBest: %i; populationWeights(pbest,gbest,rnd, phist): (%i,%i,%i,%i)", 
    maxGlobalBest, maxPersonalBest, pBestPopulationWeight, gBestPopulationWeight, rndWeight, pHistoryWeight
  );
  return result;
}

int Agent::GetNextValue(int currentMachineIndex, int level, std::vector<int> prevSelections){
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
    int populationSelector = GetRndNumberFromRange(0, rndWeight + gBestPopulationWeight + pBestPopulationWeight + pHistoryWeight + iterationBestWeight - 1);
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
      } else if (populationSelector < rndWeight + gBestPopulationWeight + pBestPopulationWeight + pHistoryWeight && personalHistorySolutions.size() >= 1){
        Solution * srcSol = selector(personalHistorySolutions);
        possibleIndex = srcSol->permutation.at(currentMachineIndex).at(level);
      } else if (populationSelector < rndWeight + gBestPopulationWeight + pBestPopulationWeight + pHistoryWeight + iterationBestWeight && iterationBestSolutions.size() >= 1){
        Solution * srcSol = selector(iterationBestSolutions);
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

int Agent::GetNextValue(int currentMachineIndex){
  std::vector<int> empty;
  return GetNextValue(currentMachineIndex, 0, empty);
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
 * returns the sum of the changeOverCost for the supplied failedFactories
 */
int Agent::GetChangeOverCost(std::vector<std::vector<int>> permutation, std::vector<int> failedFactories){
  int coCostSum = 0;
  for (unsigned int i = 0; i < permutation.size(); i++){
    if (vectorContains(failedFactories, permutation[i][0])){
      int alternativeFactoryIndex = getFirstNotInList(permutation[i], failedFactories);
      if (alternativeFactoryIndex >= 0){
        coCostSum += changeOverMatrix->GetValue(permutation[i][0], alternativeFactoryIndex);
      }
    }
  }
  return coCostSum;
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

/**
 * Iterates through the factories and calculates how much the flowDistance quality gets
 * worse if that factory fails and the changeover-cost which is introduced by the switch of the factory.
 * Each result is weighted according to the failureRate of the factory.
 * 
 * Note that this considers only the event of a single factory failing
 * 
 * altFlowDistance: new flowDistanceSum without the failed Factory f_i
 * coCost: cost to switch from failed Factory f_i to alternative f_k
 *
 * @returns sum(Factories F, 0->n, F_i-failure [0,1] * (altFlowDistance + coCost))
 */
double Agent::GetSingleFactoryFailureSum(
  int flowDistanceSum,
  std::vector<std::vector<int>> permutation
) {
  double score = 0;
  vector<int> failedFactories;
  failedFactories.push_back(0);
  for (unsigned int f_i = 0; f_i < factories.size(); f_i++){
    failedFactories[0] = f_i;
    int altFlowDistance = GetAltFlowDistanceSum(permutation, failedFactories);
    int coCost = GetChangeOverCost(permutation, failedFactories);
    #ifdef DEBUG_OUTPUT
      std::string debugOutput = string_format("Factory %i, pFailure %f, altFlowDistance %f, coCost %i", f_i, factories[f_i]->pFailure, altFlowDistance, coCost);
      debugOutput = string_format("%s score %f", debugOutput.c_str(), score);
    #endif
    if (altFlowDistance >= 0){
      score += factories[f_i]->pFailure * (altFlowDistance + coCost);
    } else {
      // TODO raise warning, add penalty
    }
    #ifdef DEBUG_OUTPUT
      printf("%s => %f\n", debugOutput.c_str(), score);
    #endif
  }
  return score;
}

double Agent::GetFailureRiskSum(std::vector<std::vector<int>> permutation){
  /**
   * Start with 0, if the failureRisk is zero (usually the QAP only case) the resulting sum will also be 0.
   **/
  double failureRiskSum = 0;
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
  sol.flowDistance = sol.flowDistanceSum / flowDistanceSumReference;
  #ifdef QAP_ONLY
    sol.quality = sol.flowDistanceSum;
  #else
    sol.failureRiskSum = GetFailureRiskSum(sol.permutation);
    sol.failureRisk = sol.failureRiskSum / failureRiskReference;
    sol.singleFactoryFailureSum = GetSingleFactoryFailureSum(sol.flowDistanceSum, sol.permutation);
    sol.singleFactoryFailure = sol.singleFactoryFailureSum / singleFactoryFailureReference;
    // aggregate the scores
    sol.quality = sol.flowDistance +sol.failureRisk + sol.singleFactoryFailure;
  #endif // QAP_ONLY
  #ifdef DEBUG_OUTPUT
  printf("\nRatedSolution %s\n", sol.ToString().c_str());
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
  printf("\nsolve with personalHistorySolutions: \n");
  for (unsigned int k = 0; k < personalHistorySolutions.size(); k++){
    printf("%i %s \n", k, personalHistorySolutions.at(k)->ToString().c_str());
  }
  #endif // DEBUG_OUTPUT

  // array of the machineIndices which need to be assigned a value for a solution
  int machineIndices[machines.size()];
  int i;
  for (i = 0; i < machines.size(); ++i){
    machineIndices[i] = i;
  }
  /**
   * randomize the order of the machineIndices to avoid a bias because of less
   * and less assignable factories during the solution creation.
   */
  #ifdef RANDOMIZE_SOLUTION_CREATION_ORDER
  std::random_shuffle(&machineIndices[0], &machineIndices[machines.size()]);
  #endif // RANDOMIZE_SOLUTION_CREATION_ORDER

  int rLevel = 0;
  // Creates the first permutation (which selects a factory for each machine)
  for (i = 0; i < machines.size(); ++i){
    sol.Set(machineIndices[i], rLevel, GetNextValue(machineIndices[i]));
  }
  /**
   * Creates the additional "permutations" to satisfy the redundancy requirement
   * for each machine. (For normal QAP instances this is not run since redundacy
   * and capacity are always 1 --> One Machine per Factory for QAP)
   */
  #ifndef QAP_ONLY
  rLevel++;
  while(rLevel < maxMachineRedundancy){
    for (i = 0; i < machines.size(); ++i){
      int currentMachineIndex = machineIndices[i];
      if (rLevel < machines[currentMachineIndex]->redundancy){
        int nextVal = GetNextValue(currentMachineIndex, rLevel, sol.permutation[currentMachineIndex]);
        int prevVal = sol.permutation[currentMachineIndex][rLevel - 1];
        sol.Set(currentMachineIndex, rLevel, nextVal);
      }
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

/**
 * updates the supplied population (inserts the solution)
 * @returns true if the solution was inserted or the population has size 0
 **/
bool UpdatePopulation(std::vector<Solution*> &population, int populationSize, Solution &inSol){
  if (populationSize < 1){
    /**
     * return true, even though the populaiton was not added to the solution
     * (the solution should still be checked against following populations)
     **/
    return true;
  }
  /**
   * If populations has max size and the solution is worse than the worst from the population
   * it wont make it into the population anyways
   */
  if (population.size() >= populationSize && inSol.quality > population[populationSize - 1]->quality){
    return false;
  }
  #ifdef IGNORE_DUPLICATE_SOLUTIONS_IN_POPULATIONS
  // check that the solution is not a duplicate
  if (containsSolution(population, inSol)){
    return false;
  }
  #endif //IGNORE_DUPLICATE_SOLUTIONS_IN_POPULATIONS
  /**
   * Iterate through the population starting with the worts solution in the population.
   * Usually the loop is exited after the first check, because the new solution is worse
   * than all solutions in the population.
   */
  int i = population.size() - 1;
  while (i >= 0){
    // smaller quality is better
    if (inSol.quality < population[i]->quality){
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

void UpdateHistory(std::deque<Solution*> &population, int populationSize, Solution &inSol){
  if (populationSize < 1){
    return;
  }
  if (population.size() >= populationSize){
    delete population.back();
    population.pop_back();
  }
  Solution* sol = new Solution(inSol);
  population.push_front(sol);
}

bool Agent::UpdatePersonalHistoryPopulation(Solution &inSol){
  #ifdef IGNORE_DUPLICATE_SOLUTIONS_IN_HISTORY
  // check that the solution is not a duplicate
  if (containsSolution(personalHistorySolutions, inSol)){
    return false;
  }
  #endif //IGNORE_DUPLICATE_SOLUTIONS_IN_HISTORY
  UpdateHistory(personalHistorySolutions, maxPersonalHistory, inSol);
  return true;
}

bool Agent::UpdateIterationBestPopulation(Solution &inSol){
  UpdateHistory(iterationBestSolutions, maxIterationBest, inSol);
  return true;
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
  UpdatePersonalHistoryPopulation(sol);
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
  Solution* sol;
  #ifdef SKIP_DUPLICATE_SOLUTIONS
  bool flag = true;
  while(flag){
    #endif // SKIP_DUPLICATE_SOLUTIONS
    sol = new Solution();
    Solve(*sol);
    #ifdef SKIP_DUPLICATE_SOLUTIONS
    if (
      containsSolution(personalHistorySolutions, *sol) ||
      containsSolution(personalBestSolutions, *sol) ||
      containsSolution(globalBestSolutions, *sol)
    ) {
      delete sol;
    } else {
      flag = false;
    }
  }
  #endif // SKIP_DUPLICATE_SOLUTIONS
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

  // check if the last iterationBestWasProvided
  if (Nan::New(Solution::constructor)->HasInstance(info[0])) {
    Solution* inSol = Nan::ObjectWrap::Unwrap<Solution>(info[0]->ToObject());
    #ifdef DEBUG_OUTPUT // disable in non debug mode for better performance
    if (inSol->quality < 0){
      return Nan::ThrowError(Nan::New("Agent::AddGlobalSolution - Solution must have a quality assigned").ToLocalChecked());
    }
    if (inSol->GetLength() != self->machines.size()){
      return Nan::ThrowError(Nan::New("Agent::AddGlobalSolution - Solution must have correct length").ToLocalChecked());
    }
    #endif // DEBUG_OUTPUT
    self->UpdateIterationBestPopulation(*inSol);
  }

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

void Agent::generateQualityScoreReferences(){
  getFlowDistanceSumReference();
  getFailureRiskReference();
  getSingleFactoryFailureReference();
  printf("\nSolution Rating References: \n flowDistanceSumReference: %f\n failureRiskReference: %f\n singleFactoryFailureReference: %f\n", flowDistanceSumReference, failureRiskReference, singleFactoryFailureReference);
}

double Agent::getFlowDistanceSumReference(){
  if (flowDistanceSumReference == -1){
    flowDistanceSumReference = (getAverageDistance() * getAverageFlow()) * (machines.size() * machines.size());
  }
  return flowDistanceSumReference;
}

double Agent::getFailureRiskReference(){
  if (failureRiskReference == -1){
    failureRiskReference = pow(getAverageFactoryFailureProbability(), getAverageMachineRedundancy()) * machines.size();
  }
  return failureRiskReference;
}

double Agent::getSingleFactoryFailureReference(){
  if (singleFactoryFailureReference == -1){
    singleFactoryFailureReference = (getFlowDistanceSumReference() + getAverageChangeOverCost()) * getAverageFactoryFailureProbability() * factories.size();
  }
  return singleFactoryFailureReference;
}

double Agent::getAverageFlow(){
  if (averageFlow == -1){
    int flowSum = 0;
    for (int i = 0; i < machines.size(); i++){
      for (int k = 0; k < machines.size(); k++){
        flowSum += flowMatrix->GetValue(i, k);
      }
    }
    averageFlow = flowSum / pow(machines.size(), 2);
  }
  return averageFlow;
}

double Agent::getAverageDistance(){
  if (averageDistance == -1){
    int distanceSum = 0;
    for (int i = 0; i < factories.size(); i++){
      for (int k = 0; k < factories.size(); k++){
        distanceSum += distanceMatrix->GetValue(i, k);
      }
    }
    averageDistance = distanceSum / pow(factories.size(), 2);
  }
  return averageDistance;
}

double Agent::getAverageChangeOverCost(){
  if (averageChangeOverCost == -1){
    int coSum = 0;
    for (int i = 0; i < factories.size(); i++){
      for (int k = 0; k < factories.size(); k++){
        coSum += changeOverMatrix->GetValue(i, k);
      }
    }
    averageChangeOverCost = coSum / pow(factories.size(), 2);
  }
  return averageChangeOverCost;
}

double Agent::getAverageFactoryFailureProbability(){
  if (averageFactoryFailureProbability == -1){
    double factoryFailureProbabilitySum = 0;
    for (int i = 0; i < factories.size(); i++){
        factoryFailureProbabilitySum += factories.at(i)->pFailure;
    }
    averageFactoryFailureProbability = factoryFailureProbabilitySum / factories.size();
  }
  return averageFactoryFailureProbability;
}

double Agent::getAverageMachineRedundancy(){
  if (averageMachineRedundancy == -1){
    int machineRedundancySum = 0;
    for (int i = 0; i < machines.size(); i++){
        machineRedundancySum += machines.at(i)->redundancy;
    }
    averageMachineRedundancy = machineRedundancySum / machines.size();
  }
  return averageMachineRedundancy;
}
