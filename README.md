# Muon
This is a rendering framework built to help in the initial setup of simulations.

## Details
This project is built using MSVC with the Visual Studio 2022 toolset (v143) for the C++20 standard.

## Dependencies
Part of the point of making this project was to attempt to create a basic rendering system while using as few libraries as possible. While I'm still committed to this goal, certain features I would like to implement are too impractical to try to learn and create myself while still focusing my own growth in graphics programming specifically, such as Audio or Online Play. The following is a list of the external libraries I'll be making use of and for what purpose.
* [DirectX Toolkit 2017](https://github.com/microsoft/DirectXTK) (NuGet)
  * Reading image files for texture generation
* [Assimp 3.0.0](http://www.assimp.org/) (NuGet)
  * Loading 3D Models
  
## Screenshots
![Normal Mapping Demo](https://github.com/rubenaryo/Easel/blob/master/images/screen1.PNG?raw=true)

## Visual Studio Configuration
The development environment used is Visual Studio 2022, configured such that
* Intermediate obj files get placed in the _compile folder.
* Executables get placed in _generated.
