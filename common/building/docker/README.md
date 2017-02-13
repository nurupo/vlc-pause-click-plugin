## Cross-compiling to Windows with Docker

### Intro

It's a lot easier to build Windows plugin binaries using Docker than following [the build instructions](../build.md), as everything is already automated for you.
That's also the way I build Windows binaries that I release.

The Dockerfile provided in this directory sets up an image that will build all versions of the plugin: 2.1.x and 2.2.x, both 32-bit and 64-bit ones.
Once the image is created, you don't have to re-create it if you change plugin source code, it will pickup any changes in code you make.

### Building

I assume you are running on Linux and you already have Docker installed.

Note that you will need at least 3GiB of free disk space, as well as about 20 minutes of time, which varies greatly depending on CPU and Internet connection bandwidth.
You will have to download about 1GiB of data to build the image.

First we need to create the image. You can use the Dockerfile provided directly

```bash
sudo docker build -t vlc-pause-click-plugin-windows-build .
```

or use the image from my DockerHub (see next) build off this Dockerfile, which should be faster than using the Dockerfile directly.

After the image is successfully built, we can run it to build the binaries.

Note that the build script expects root of this, vlc-pause-click-plugin, git repository to be mounted under `/repo` and the directory into which you want the binaries to be put to be mounted under `/build` inside of the container.
For this, we use the `-v` flag when running a container based on the image. The syntax is `-v <host-directory-path>:<container-mounting-point>`.

This is how you would run the container based on the image. Don't forget to specify your own paths in `-v` flags.

```bash
sudo docker run --rm \
    -v /home/nurupo/git/vlc-pause-click-plugin:/repo \
    -v /home/nurupo/git/vlc-pause-click-plugin/build:/build \
    vlc-pause-click-plugin-windows-build
```

To run a container based off my DockerHub image (which will also pull the image if you don't have it yet), run

```bash
sudo docker run --rm \
    -v /home/nurupo/git/vlc-pause-click-plugin:/repo \
    -v /home/nurupo/git/vlc-pause-click-plugin/build:/build \
    nurupo/vlc-pause-click-plugin-windows-build
```

This will start the building process. You should see the binaries getting created in the directory you specified to be used for `/build` mounting point.

### Bulding only selected versions of the plugin

If for some reason you don't want to build all of the 4 versions of the plugin binaries, you can comment out the building of unneeded binaries in `script.sh` inside of the container by running

```bash
sudo docker run -it \
    -v /home/nurupo/git/vlc-pause-click-plugin:/repo \
    -v /home/nurupo/git/vlc-pause-click-plugin/build:/build \
    vlc-pause-click-plugin-windows-build \
    /bin/bash
```

(replace `vlc-pause-click-plugin-windows-build` to `nurupo/vlc-pause-click-plugin-windows-build` if you use my DockerHub image)

command, installing an editor of your choice

```bash
apt-get update && apt-get install nano -y
```

commenting out the undeeded builds

```bash
nano script.sh
```

starting the build script

```bash
./script.sh
```

and stopping the container after the build is done

```bash
exit
```

### Freeing space

To free the space taken by the image, you can run `sudo docker images` to get the list of images and `sudo docker rmi` to remove an image.
There is also `sudo docker ps -a` to get the list of containers and `sudo docker rm` to remove a container, in case you ran `/bin/bash` in the container.
