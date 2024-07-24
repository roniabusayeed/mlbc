# mlbc: Media Labeller for Binary Classification

mlbc is a tool designed to efficiently label media (images or audio) for the purpose of binary classification in machine learning projects.

# Build & Install (macOS)
 1. Install [Homebrew](https://brew.sh/).
 2. Clone the repository `$ git clone --recursive https://github.com/roniabusayeed/mlbc.git` ('$' is not part of the command. It represents your terminal prompt)
 3. Run the following commands from the project root directory (i.e., `.../mlbc/`).
    - `$ ./configure.sh`. You only need to run this command once even if you build multiple times.
    - `$ ./build.sh` This will build the project executable. You can find it under `your-project-root/build` directory.
	- `$ ./install.sh` This will install the progam executable on your system.

## Keyboad Shortcuts
 - Press the keys A and D to decrease and increase the slider value respectively. You can adjust the sensitivity using the sensitivity slider (the slider below the primary slider).
 - Press the key L to label (this has the same effect of pressing the button "Label" )

 **Important**: Currently, only Image files are supported. 