APP_NAME := compilerbook
.DEFAULT_GOAL := help

.PHONY: docker.build
docker.build: ## Build docker image
	docker build -f ./Dockerfile -t $(APP_NAME):latest .

.PHONY: docker.run
docker.run: ## Run on docker
	docker run -it --rm $(APP_NAME):latest /bin/bash

.PHONY: docker.9cc.test
docker.9cc.test: docker.build
	docker run -it --rm $(APP_NAME):latest make test

.PHONY: help
help: ## Show options
	 @grep -E '^[a-zA-Z_-{\.}]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-15s\033[0m %s\n", $$1, $$2}'
