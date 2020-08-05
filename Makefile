.PHONY: help

help: ## this help
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

docker-build:
	docker build --tag registry.leoek.tech/rqap:$(shell git rev-parse HEAD) --target prod .

docker-publish: docker-build
	docker push registry.leoek.tech/rqap:$(shell git rev-parse HEAD)
	docker tag registry.leoek.tech/rqap:$(shell git rev-parse HEAD) registry.leoek.tech/rqap:current
	docker push registry.leoek.tech/rqap:current

docker-dev-build: 
	docker build --tag rqapdev --target dev .

start: docker-dev-build
	docker run --rm -it --name rqapdev -v ${PWD}/src:/user/src/app/src -v ${PWD}/problems:/usr/src/app/problems rqapdev

docker-irace-build: 
	docker build --tag registry.leoek.tech/rqapirace:$(shell git rev-parse HEAD) --target irace .

docker-irace-publish: docker-irace-build
	docker push registry.leoek.tech/rqapirace:$(shell git rev-parse HEAD)
	docker tag registry.leoek.tech/rqapirace:$(shell git rev-parse HEAD) registry.leoek.tech/rqapirace:current
	docker push registry.leoek.tech/rqapirace:current

irace-clean:
	rm -rf ./irace/output
	mkdir -p ./irace/output
	touch ./irace/output/.gitkeep
	rm -rf ./irace/std
	mkdir -p ./irace/std
	touch ./irace/std/.gitkeep

irace-check:
	cd irace && Rscript checkIrace.R
