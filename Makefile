.PHONY: help

help: ## this help
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}'

rebuild: 
	docker build -t rqapdev .

start: rebuild
	docker run --rm -it --name rqapdev -v ${PWD}/src:/user/src/app/src -v ${PWD}/problems:/usr/src/app/problems rqapdev
