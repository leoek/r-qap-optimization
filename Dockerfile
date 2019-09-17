#FROM node:12.10.0-stretch
#FROM node:6.7.0
FROM node:8.16.1-jessie

ARG uid=1000
ARG gid=1000

RUN apt-get update -y && apt-get install -y make \
    gcc \
    build-essential \
    python python-pip && \
    rm -rf /var/lib/apt/lists/*

RUN npm install -g node-gyp

# update user and group id
RUN usermod -u $uid -o node \
    && groupmod -g $gid -o node && \
    chown -R $uid:$gid /home/node

RUN mkdir -p /usr/src/app && \
    chown -R node:node /usr/src
USER node
WORKDIR /usr/src/app

COPY package.json .
COPY yarn.lock .

RUN yarn

COPY binding.gyp .

COPY src ./src
RUN yarn setup
COPY problems ./problems
USER root
RUN chown -R node:node /usr/src
USER node

ENTRYPOINT [ "yarn" ]
CMD [ "start" ]