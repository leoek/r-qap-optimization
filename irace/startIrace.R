library(irace)
system.file(package="irace")

parameters <- readParameters("./parameters.txt")
scenario <- readScenario(filename = "./scenario.txt", scenario = defaultScenario())

irace(scenario = scenario, parameters = parameters)
