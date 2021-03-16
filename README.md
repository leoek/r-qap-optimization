# Redundant Quadratic Assignment Problem Optimization

## Usage

An `instanceType` must always be specified. The `instanceType` can either be defined as `QAP` or `RQAP`.

The problem instance must be supplied in the problems folder with the file extension `.dat` for QAP instances and `.rqap` for RQAP instances. The name of the instance to optimize must always be specified as `instanceName`, without the file extension.

### Using the prebuild docker image:

There are some instances prebuilt into the docker image, which can be used directly. They can be checked in this repositories [problems folder](https://github.com/leoek/r-qap-optimization/tree/master/problems).

If you want to supply other instances, these instances should be mounted into the docker container at `/usr/src/app/problems`. For example to mount instances from `/tmp/instances` the following flag for the docker run command should be used: `-v tmp/instances:/usr/src/app/problems:ro`.

- command: `docker run --rm leoek/rqap:current serve:node default <instanceName> <instanceType> <arguments>`

- example: `docker run --rm leoek/rqap:current serve:node default nug12b RQAP --agents 10 --solutionCountTarget 100000`

- example with custom instances from `/tmp/instances`: `docker run --rm -v tmp/instances:/usr/src/app/problems:ro leoek/rqap:current serve:node default nug12b RQAP --agents 10 --solutionCountTarget 100000`

### Using a local build:

- Build the project
  1. donwload dependencies: `yarn`
  2. build the project: `yarn build`
- run: `yarn serve:node default <instanceName> <instanceType> <named arguments>`
- example: `yarn serve:node default nug12b RQAP --agents 10 --solutionCountTarget 100000`

## Parameters

- Number of Agents: `agents`
- Number of Solutions to create (execution will stop when this number reached): `solutionCountTarget`
- `Number of Solutions to create during warmup (defaults to 100):`warmupSolutions`
- Size of the personal best Population: `maxPersonalBest`
- Size of the global best Population: `maxGlobalBest`
- Size of the personal history Population: `maxPersonalHistory`
- Size of the iteration best Population: `maxIterationBest`
- Weight of the personal best Population: `pBestPopulationWeight`
- Weight of the global best Population: `gBestPopulationWeight`
- Weight for a random selection: `rndWeight`
- Weight of the personal history Population: `pHistoryWeight`
- Weight of the iteration best Population: `iterationBestWeight`
- Directory to read problem instances from (defaults to `./problems`): `problemInstancesDirectory`
- Activate some per agent randomization of populations sizes and weights (0 - `false`, 1 - `true`, defaults to `false`): `randomizeAgentOptions`

## Development Setup

### Requirements

1. c and c++ compiler and debugger
2. node >= 6.7
3. globally installed node-gyp (`yarn global add node-gyp`)
