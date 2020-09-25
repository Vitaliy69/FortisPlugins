# Plugins for fourth generation Fortis satellite receivers (STi H205)

**Prepare the toolchains**

```
mkdir -p src
cd src
wget https://www.dropbox.com/s/bfas3u8cc0av77k/CrossCompiler.tar.gz
tar -zxvf CrossCompiler.tar.gz
```

Add the cross compilers binary path to a system _PATH_ variable _export PATH=$PATH:~/src/sh4-unknown-linux-gnu/bin_, alternatively, add this command to _~/.bashrc_ file.

**Compile the plugins**

```
git clone https://github.com/Vitaliy69/FortisPlugins.git
cd FortisPlugins/plugins/
./build.sh
```

Check for output binaries the directory _FortisPlugins-bin_ in your home path.
