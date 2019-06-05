
export const getParametersFromArgs = () => {
    const parameters = {
        instanceName: "default",
        agents: 10
    }
    process.argv.forEach((val, i) => {
        // 0 and 1 are node and the js filename
        if (i === 2){
            parameters.problemInstance = val;
        } else if (i === 3) {
            parameters.agents = parseInt(val);
        }
    })
    return parameters;
}

export default getParametersFromArgs;