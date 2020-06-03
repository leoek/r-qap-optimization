#include "Agent.h"

using namespace v8;
using namespace std;

Nan::Persistent<v8::FunctionTemplate> Agent::constructor;

NAN_MODULE_INIT(Agent::Init) {
  v8::Local<v8::FunctionTemplate> ctor = Nan::New<v8::FunctionTemplate>(Agent::New);
  constructor.Reset(ctor);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(Nan::New("Agent").ToLocalChecked());

  // link our getters and setter to the object property
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("x").ToLocalChecked(), Agent::HandleGetters, Agent::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("y").ToLocalChecked(), Agent::HandleGetters, Agent::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("z").ToLocalChecked(), Agent::HandleGetters, Agent::HandleSetters);
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("intarray").ToLocalChecked(), Agent::HandleGetters, Agent::HandleSetters);

  Nan::SetPrototypeMethod(ctor, "addGlobalSolution", AddGlobalSolution);
  Nan::SetPrototypeMethod(ctor, "createSolution", _CreateSolution);

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
  if (inSol->quality <= 0){
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
  Solution* newSol = new Solution(*inSol);
  // add the copy to the population
  bool added = self->UpdateGlobalPopulation(*newSol);

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
  if (propertyName == "x") {
    info.GetReturnValue().Set(self->x);
  } else if (propertyName == "y") {
    info.GetReturnValue().Set(self->y);
  } else if (propertyName == "z") {
    info.GetReturnValue().Set(self->z);
  } else if (propertyName == "intarray") {
    v8::Local<v8::Array> jsArray = Nan::New<v8::Array>(self->intarray.size());
    for (size_t i = 0; i < self->intarray.size(); i++)
    {
        printf("value %lu %d", i, self->intarray[i]);
        Nan::Set(jsArray, i, Nan::New<Number>(self->intarray[i]));
    }
    info.GetReturnValue().Set(jsArray);
  } else {
    info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_SETTER(Agent::HandleSetters) {
  Agent* self = Nan::ObjectWrap::Unwrap<Agent>(info.This());

  if(!value->IsNumber()) {
    return Nan::ThrowError(Nan::New("expected value to be a number").ToLocalChecked());
  }

  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "x") {
    self->x = value->NumberValue();
  } else if (propertyName == "y") {
    self->y = value->NumberValue();
  } else if (propertyName == "z") {
    self->z = value->NumberValue();
  } else if (propertyName == "intarray"){

  }
}

int Agent::GetNextValue(){
  Machine * currentMachine = machines.at(currentMachineIndex);
  // #PERFORMANCE
  // check the capacity requirement after selecting randomly
  // remove from availableFactories if not enough space
  // and select randomly again
  vector<int> availableFactories;
  for (unsigned int i = 0; i < factories.size(); i++){
    if (factories.at(i)->GetUnusedCapacity() >= currentMachine->size){
      availableFactories.push_back(i);
    }
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
        if (currentMachineIndex >= srcSol->permutation.size()){
          printf("selected invalid srcSol from global best %i / %i, srcSol: %s \n", currentMachineIndex, srcSol->permutation.size(), srcSol->ToString().c_str());
        } else {
          possibleIndex = srcSol->permutation.at(currentMachineIndex);
        }
      } else if (populationSelector < rndWeight + gBestPopulationWeight + pBestPopulationWeight && personalBestSolutions.size() >= 1){
        Solution * srcSol = selector(personalBestSolutions);
        if (currentMachineIndex >= srcSol->permutation.size()){
          printf("selected invalid srcSol from local best %i / %i, srcSol: %s \n", currentMachineIndex, srcSol->permutation.size(), srcSol->ToString().c_str());
        } else {
          possibleIndex = srcSol->permutation.at(currentMachineIndex);
        }
      }
      // Check whether there is a possible index which was copied from a previous Solution
      if (possibleIndex >= 0 && possibleIndex < factories.size()){
        // Check whether the machine fits into that factory (get the factory at that index)
        // select that index if it is feasible
        if (possibleIndex >= 0 && factories.at(possibleIndex)->GetUnusedCapacity() >= machines.at(currentMachineIndex)->size){
          selected = possibleIndex;
        }
      }
    }
  }
  // Use the size of currenMachine in the chosen factory
  factories.at(selected)->UseCapacity(currentMachine->size);
  return selected;
}

void Agent::ResetFactories(){
  vector<Factory*> newFactories;
  for (unsigned int i = 0; i < factories.size(); i++){
    factories.at(i)->ResetUsedCapacity();
  }
}

int Agent::RateSolution(Solution &sol){
  // Rate the generated solution
  // #TODO rate all criteria (currently only flow*distance)
  int flowDistanceSum = 0;
  for (unsigned int m_i = 0; m_i < machines.size(); m_i++){
    for (unsigned int m_k = 0; m_k < machines.size(); m_k++){
      int flow = flowMatrix->GetValue(m_i, m_k);
      int distance = distanceMatrix->GetValue(sol.permutation[m_i], sol.permutation[m_k]);
      flowDistanceSum += flow * distance;
    }
  }
  sol.quality = flowDistanceSum;
  return flowDistanceSum;
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
  currentMachineIndex = 0;
  while(currentMachineIndex < machines.size()){
    sol.Add(GetNextValue());
    currentMachineIndex++;
  }
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
   * To allow efficiently freeing memory of solutions which drop out of the population
   * a copy of the solution is added to the population.
   * - This ensures that different populations do not have references to the same solution.
   * - Also it is assured that solutions which did not make it into the population can be
   *   deleted
   */
  Solution* sol = new Solution(inSol);
  /**
   * Iterate through the population starting with the worts solution in the population.
   * Usually the loop is exited after the first check, because the new solution is worse
   * than all solutions in the population.
   */
  int i = population.size() - 1;
  while (i >= 0){
    // smaller quality is better
    if ( sol->quality < population[i]->quality){
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

Local<Object> CreateWrappedSolution(Solution &sol){
  // Create new wrapped solution instance _sol
  v8::Local<v8::Array> jsArray = Nan::New<v8::Array>(sol.permutation.size());
  for (size_t i = 0; i < sol.permutation.size(); i++)
  {
      Nan::Set(jsArray, i, Nan::New<Number>(sol.permutation[i]));
  }

  const int argc = 2;
  v8::Local<v8::Value> argv[argc] = {
    jsArray,
    Nan::New(sol.quality)
  };

  Local<Function> constructorFunc = Nan::New(Solution::constructor)->GetFunction();
  Local<Object> _sol = Nan::NewInstance(constructorFunc, argc, argv).ToLocalChecked();
  return _sol;
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

NAN_METHOD(Agent::_CreateSolution) {
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
