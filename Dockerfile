FROM node:20 AS build
WORKDIR /app/
COPY package.json package-lock.json /app/
RUN npm install
COPY tsconfig.json webpack.config.js /app/
COPY dist/index.html dist/favicon.svg dist/otto.*.bin /app/dist/
COPY src /app/src
RUN npx webpack

FROM nginx:1.25.1
COPY --from=build /app/dist /web
COPY nginx.conf /etc/nginx/nginx.conf
