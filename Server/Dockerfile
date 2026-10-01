FROM node:22-alpine
RUN apk add --no-cache tzdata
WORKDIR /app
COPY package.json package-lock.json ./
RUN npm ci --omit=dev --no-audit --no-fund
COPY src ./src
COPY public ./public
EXPOSE 3000
CMD ["node", "src/server.js"]
