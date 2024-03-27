# Raytracer 1d
Author: Sam Martin (MART6353)
ID: 5835125

## Compilation
Just run `make` to compile the program. The executable will be in the __./out__ directory

## Running
- Use `./out/raytracer1d {input file}`
  - or whatever the path is to the out directory
- Use `make run INPUT={inputfile}`
  - this will run from the path data, so if using this command, your input file must be in the data directory or you must specify the relative path to the data directory
- Use `make test`
  - This will run a handful of files at the same time from the data directory. The name and amount of files can be specified from the config/config.default.mk file
  - They must be in the `{filename}{1 indexed number}.{file ext}` format
  - This is the only command you will need to modify the config file for
  - It is currently set up to run the three example files given which are currently in the data directory

### Textures
Textures must be in a texture directory where the input files are located. 

If your input files are in `./data`, the texture files must live in `./data/texture`. This is currently not able to be changed.

## Showcase image
The showcase image is labelled `aloe.jpeg`. You are free to run this yourself with `make run INPUT=aloe.obj`, but be warned it takes a ***VERY*** long time.

## Known Issues
Currently, there is an issue with loading textures that has persisted since hw1c. I am going to attend an office hours to get that fixed, but I'd rather submit this now than wait to figure out what the issue is to add it here. 
