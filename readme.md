<div align="center">
    <h1>Switchseerr</h1>
    <p>Switchseerr is a third-party client for Jellyseerr for multiple platforms</p>
</div>

<p align="center">
    <a rel="LICENSE" href="https://github.com/PoloNX/Switchseerr/blob/master/LICENSE">
        <img src="https://img.shields.io/static/v1?label=license&message=MIT&labelColor=111111&color=0057da&style=for-the-badge&logo=data%3Aimage/png%3Bbase64%2CiVBORw0KGgoAAAANSUhEUgAAABQAAAATCAYAAACQjC21AAAACXBIWXMAAAsTAAALEwEAmpwYAAAAIGNIUk0AAHpFAACAgwAA/FcAAIDoAAB5FgAA8QEAADtfAAAcheDStWoAAAFGSURBVHjarJK9LgRhFIafWUuiEH/rJwrJClEq3IELUKgo3IrETWh0FC7BNVih0AoKBQoEydq11qMwm5yMsbPEm3yZd55zvnfO92VQKVhLak09UZeL%2BrsVZ9Qdv2tXnf1NYEndUushZFGthvemuq32FwWuq%2BeZid5DvZGpXambeYGr6qnd9dGldqaudQL3QuFWvVbbmaC6%2BprDr9WbwA4SdQW4BwaABb50CTykfjjwC%2BAx9SPAfOANYDxRCXpOnxNAM4ePA63Ul8NHR4E2QClsGgGG0jUR%2BFjglcAn8/pj4HTwUz/42FPJ68lOSDhCkR/O46XM0Qh3VcRH83jph%2BZefKUosBr8XA%2B%2BmufLAR4Dh6k/CrzWA691YOc/3Ejv6iNM3k59Xw%2B8D3gC9hN1ErjjfzSbqHVg8J8CG2XgBXgL4/9VCdD6HACaHdcHGCRMgQAAAABJRU5ErkJggg%3D%3D" alt=License>
    </a>
    <a rel="VERSION" href="https://github.com/PoloNX/AtmoPackUpdater">
        <img src="https://img.shields.io/static/v1?label=version&message=1.0.0&labelColor=111111&color=06f&style=for-the-badge" alt="Version">
    </a>
    <a rel="BUILD" href="https://github.com/PoloNX/Switchseerr/actions">
        <img src="https://img.shields.io/github/actions/workflow/status/PoloNX/Switchseerr/c-cpp.yml?branch=master &labelColor=111111&color=06f&style=for-the-badge" alt=Build>
    </a>
</p>

---

- [Features](#features)
- [Screenshots](#screenshots)
- [How to build](#how-to-build)
- [Todo](#todo)
- [Thanks to](#thanks-to)
- [License](#license)


## Features
- Connect to multiple Jellyseerr servers and multiple users
- Request movies and TV shows

## Screenshots

![](./screenshots/1.png)

<details>
    <summary>More screenshots</summary>
    <img src="https://raw.githubusercontent.com/PoloNX/Switchseerr/master/screenshots/1.png" alt="Screenshot 1">
    <img src="https://raw.githubusercontent.com/PoloNX/Switchseerr/master/screenshots/2.png" alt="Screenshot 2">
    <img src="https://raw.githubusercontent.com/PoloNX/Switchseerr/master/screenshots/3.png" alt="Screenshot 3">
    <img src="https://raw.githubusercontent.com/PoloNX/Switchseerr/master/screenshots/4.png" alt="Screenshot 4">
</details>

## How to build

This project uses [Xmake](https://xmake.io/) as its build system. To build the project, follow these steps:

### Desktop

1. Install [Xmake](https://xmake.io/#/getting_started) on your system.
2. Clone the repository:
   ```bash
   git clone https://github.com/PoloNX/Switchseerr.git
   cd Switchseerr
   ```
3. Build the project:
    ```bash
   xmake
   ```
The built binary will be located in the `build` directory.
If you want to run it directly, you can use:
   ```bash
   xmake run
   ```

### Nintendo Switch
1. Install [Xmake](https://xmake.io/#/getting_started) on your system.
2. Install [DevkitPro](https://devkitpro.org/wiki/Getting_Started) and set up the environment.
3. Install switch dependencies:
   ```bash
   pacman -S switch-curl switch-zlib switch-glfw switch-mesa switch-glm
   ```
4. Clone the repository:
   ```bash
   git clone https://github.com/PoloNX/Switchseerr.git
   cd Switchseerr
   ```
5. Build the project:
   ```bash
   xmake f --yes -p cross -a aarch64 --toolchain=devkita64
   xmake
   ```
The built binary will be located in the `build` directory.
If you want to run it on your switch with nxlink you can use:
   ```bash
   xmake run SwitchSeerr --nx=<ip_address>
   ```

## Todo

### Platform Support
- [ ] **PlayStation 4** - Add PS4 platform support
- [ ] **Apple TV** - Add Apple TV platform support  
- [ ] **PlayStation Vita** - Add PSV platform support

### Future Enhancements
- [ ] Additional Jellyseerr features integration
- [ ] Lidarr integration
- [ ] Support for Overseerr

## Thanks to
- **[xfangfang](https://github.com/xfangfang) for [borealis](https://github.com/xfangfang/borealis) and [wiliwili](https://github.com/xfangfang/wiliwili)**
- **[dragonflylee](https://github.com/dragonflylee) for [swiftfin](https://github.com/dragonflylee/swiftfin)**
- [fallenbagel](https://github.com/fallenbagel) for [jellyseerr](https://github.com/fallenbagel/jellyseerr)

## License
This project is licensed under the MIT License. See the [LICENSE](https://github.com/PoloNX/Switchseerr/blob/master/LICENSE) file for more information.