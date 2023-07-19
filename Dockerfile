FROM python:3.11.2 AS base
RUN apt-get update && apt-get install -y wget bzip2
RUN wget -q https://github.com/pyodide/pyodide/releases/download/0.23.4/pyodide-core-0.23.4.tar.bz2 &&\
    tar -xjf pyodide-core-0.23.4.tar.bz2 && mv /pyodide /web && rm pyodide-core-0.23.4.tar.bz2 &&\
    rm /web/pyodide.mjs /web/package.json

ENV EMSDK=/opt/emsdk EMSDK_NODE=/opt/emsdk/node/14.18.2_64bit/bin/node \
    PATH=/opt/emsdk:/opt/emsdk/upstream/emscripten:/opt/emsdk/node/14.18.2_64bit/bin:$PATH
RUN git clone https://github.com/emscripten-core/emsdk.git $EMSDK &&\
    emsdk install 3.1.32 && emsdk activate 3.1.32 && pip install pyodide-build==0.23.4 swig==4.1.1

RUN wget -q https://cdn.jsdelivr.net/pyodide/v0.23.4/full/numpy-1.24.2-cp311-cp311-emscripten_3_1_32_wasm32.whl -P /web/

RUN git clone https://github.com/szabolcsdombi/zengl -b 1.13.0 /zengl
RUN pyodide build /zengl -o /web/

COPY modules/canvas /canvas
RUN pyodide build /canvas -o /web/

FROM node:20 AS build
WORKDIR /app/
COPY package.json package-lock.json /app/
RUN npm install
COPY tsconfig.json webpack.config.js /app/
COPY dist/index.html dist/favicon.svg dist/otto.*.bin /app/dist/
COPY src /app/src
RUN npx webpack

FROM nginx:1.25.1
COPY --from=base /web /web
COPY --from=build /app/dist /web
COPY nginx.conf /etc/nginx/nginx.conf
