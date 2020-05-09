# SuperSonicML
[Bakkesmod plugin](https://bakkesmod.com) that supplies inputs to rocket league at a very high speed.

## Setup (For developers)
Download [CMake v3.16.6](https://cmake.org/download/) or higher (Released 10th of April, 2020)

Download the latest [LibTorch](https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-1.5.0.zip) version (Tested with 1.5.0, cpu version)

For MSVC:
```commandline
git clone https://github.com/GodGamer029/SuperSonicML.git
cd SuperSonicML
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\your\path\to\libtorch ..
cmake --build . --config Release
```

If you use CLion, you need to add the command line parameter `-DCMAKE_PREFIX_PATH` to your cmake profile in Settings -> Build, Execution, Deployment -> CMake. You also need to change Build Type to release.

Find the bakkesmod plugin settings folder (Usually located in `rocketleague\Binaries\Win64\bakkesmod\plugins`) and drop [this file](https://raw.githubusercontent.com/GodGamer029/SuperSonicML/master/resources/supersonicml.set) in there.

If you have an IDE, you can use [this python script](https://pastebin.com/cs6i2gQD) to automatically reload the plugin as the post-build step without having to restart rocket league.
Replace the `bakkesmod_plugin_folder` variable in that script with the path to your bakkesmod plugins folder, and supply the path to the SuperSonicML.dll as the first argument.

Example:
```commandline
python C:\Users\<username>\source\repos\SuperSonicML\bakkes_patchplugin.py build\bin\SuperSonicML.dll
```

If you, for some reason, cannot use this script, you need to manually copy the SuperSonicML.dll from the output directory to the bakkesmod plugins folder, and reload the plugin using the plugin manager inside bakkesmod.

## Running the plugin
#### Step 1:
Install and open [bakkesmod](https://bakkesmod.com) 

#### Step 2:
Download a 64-bit injector (I used Xenos64) and inject these dlls from the libtorch/lib directory into the rocketleague.exe process
 - asmjit.dll
 - c10.dll
 - fbgemm.dll
 - libiomp5md.dll
 - libiompstubs5md.dll
 - torch.dll
 - torch_cpu.dll
 - torch_global_deps.dll
 
You may need to injector more dlls depending on your version of libtorch (I used libtorch v1.5.0, cpu version)
 
If you get an error saying that you can't inject 64-bit dlls into a 32-bit process, you have chosen the RocketLeague launcher (32-bit) instead of the actual RocketLeague game (64-bit). Note that both processes have the exact same name (rocketleague.exe)

These dlls have to be injected everytime you start rocketleague and intend to use this plugin, it won't load if those dlls are not loaded.

Todo: remove this step

#### Step 3:

Either manually copy the dll and enable it in plugin manager, or run the python script as described in [Setup](#setup-for-developers)

