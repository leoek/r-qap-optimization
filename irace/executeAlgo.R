args = commandArgs(trailingOnly=TRUE)

# einfaches Beispiel:
# zwei Parameter werden in den Algorithmus eingesetzt
# Zuweisung erfolgt in R mit "<-" oder "="

# Argumente ausgeben
# Argumente sollten mindestens sein:
# Probleminstanz, Parameter des Algorithmus
for (i in 1:length(args)){
    tempString <- paste("Das ", i, ". Argument ist: ", args[i], sep="")
    print(tempString)
}

# Namen der Instanz bestimmen 
# (was hier im Detail passiert: args[1] beinhaltet den vollständigen Pfad
# zur Instanz. Man sucht im String die "/"-Symbole und extrahiert
# den String hinter dem letzten "/"
pathToInstance <- args[1]
slashPositions <- gregexpr(pattern="/", pathToInstance)[[1]]
lastSlashPosition <- slashPositions[length(slashPositions)]
instanceName <- substring(pathToInstance, lastSlashPosition + 1, nchar(pathToInstance))

# Parameter anhand der Argumente etzen
agents <- 10
solutionCountTarget <- "10000"

# append all args except the path to the instance
combinedArgs <- paste(args[-1], collapse = " ")
combinedArgsForPath <- paste(args[-1], collapse = "_")
outputFileName <- paste("outputForIrace", instanceName, combinedArgsForPath, sep="_")

outDirMount <- paste(getwd(),"/output:/usr/src/app/out:rw", sep = "")
instancesDirMount <- paste(getwd(),"/instances:/usr/src/app/problems:ro", sep = "")

# Den String-Befehl zum Starten des Algorithmus mit Parametern zusammensetzen
combinedString <- paste("docker run --rm -v", outDirMount, "-v", instancesDirMount, "registry.leoek.tech/rqap:current serve:node irace", instanceName, "RQAP --qualityTarget 0 --agents", agents, "--solutionCountTarget", solutionCountTarget, "--iraceOutputFileName", outputFileName, combinedArgs, sep=" ")
print(combinedString)

#startTime <- Sys.time()

# HIER WIRD DER ALGORITHMUS AUFGERUFEN
system(combinedString) # system: führt Shell-Befehl aus

#endTime <- Sys.time()

#output lesen
outputFilePath <- paste("./output/", outputFileName, sep="")
result <- readLines(outputFilePath)

# irace liest die erreichte Lösungsqualität am Ende anhand eines echo-Befehls
# ab, also muss noch ein echo gemacht werden.
# Es ist auch möglich, dass irace nach Laufzeit optimiert (siehe Dokumentation
# von irace für mehr Infos)

system(paste("echo", result))
