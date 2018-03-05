# Spore frontend javascript

WIP frontend view and browser for the Spore RFID scanner server and hardware

Built with [svelte](https://svelte.technology/) - self-compiling JS library

# Structure

```
public          # compiled files (html,js,css)
src
├── App.html    # Main entry and router
├── components  # Reusable components
└── pages       # Layout and API conn
```

# TODO

dependency svelte-router fix for customizable path regexp - [pending pull request](https://github.com/jikkai/svelte-router/pull/8)

for now we compile from [fork](https://github.com/bensinober/svelte-router)


# Development

You'll need nodejs and yarn (or npm)

```
yarn install
```

Workaround for building modified router

```
cd node_modules/svelte-router && yarn install && yarn run build & cd ../..
```

```
yarn run dev
```

# Deploy

To compile bundle and js:

```
yarn install && yarn run build
```

or simply `make build`