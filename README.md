![XNetProfiles Logo](XNetProfilesLogo.png)

**XNetProfiles** is a network configuration manager for the original Xbox console. 
It allows users to create, manage, and switch between multiple network profiles (e.g., for different LAN environments or DNS setups) and applies them directly to the `evox.ini` configuration file.
At this time, this tool should only be used by people utilizing the **Evolution X version 3935 dashboard**, as it is the only one I have built support for.
Support for others may be added in the future.

Developed with [nxdk](https://github.com/XboxDev/nxdk).

## Features

- **Profile Management**: Create up to 10 custom network profiles.
- **Custom IP Configuration**: Manually edit IP address, Subnet Mask, Gateway, DNS 1, and DNS 2 for each profile.
- **Evolution X Compatibility**: Patches your `evox.ini` automatically. It backs up your original file as `evox.ini.bak` before making changes.
- **Persistent Settings**: Remembers your custom `evox.ini` path across sessions, and keeps your profiles saved in E:\
- **File Explorer**: Integrated file browser to locate your `evox.ini` on any partition (`C:`, `E:`, `F:`, `G:`).

## Navigation

- **DPAD**: Move selection / Change values.
- **A**: Select / Confirm / Edit Name.
- **B**: Back / Cancel.
- **X**: Edit selected profile / increment up.
- **Y**: Delete selected profile / increment down.
- **START**: Save profile (when editting a profile).

## Demonstration

https://github.com/user-attachments/assets/b764ae76-7dc9-4a3b-913b-2bd726299cd7

## Setup & Installation

### Prerequisites

- A softmodded or hardmodded original Xbox **WITH** Evolution X Dashboard installed
    - **ONLY** been tested with version 3935
- `evox.ini` must be present on your console (standard locations like `C:\`, `E:\`, or dashboard folders are searched automatically).
    - You can point to where your evox.ini is located, if its in a different location for whatever reason.

### Building from Source

1. Install the [nxdk SDK](https://github.com/XboxDev/nxdk).
2. Clone this repository.
3. Open a terminal in the repository directory.
4. Edit `Makefile` so that NXDK_DIR points to your nxdk installation
```bash
NXDK_DIR ?= /home/USER/nxdk
```
5. Edit `make.sh` so that it contains your path to the activate file
```bash 
eval "$(/home/USER/nxdk/bin/activate -s)"
```
6. Run `make.sh`, and customize its path to activate nxdk. (refer to [this:](https://github.com/XboxDev/nxdk/wiki/Build-a-Sample))
7. The output `default.xbe` will be in the `bin/` directory, put this into your apps directory on your custom dashboard.

### Installation
1. Rename `bin/` directory to `XNetProfiles/`
2. FTP the contents of renamed directory from step 1 to the Apps folder on your Evolution X dashboard (`E:\Apps\XNetProfiles\` in my case).
3. Run `XNetProfiles` from your dashboard.

## Acknowledgments

- Built using **[nxdk](https://github.com/XboxDev/nxdk)** and **SDL2**.
- Background image from: [https://www.reddit.com/r/RG353M/comments/10076jr/wallpapers_selection_640x480_enjoy/](https://www.reddit.com/r/RG353M/comments/10076jr/wallpapers_selection_640x480_enjoy/)

## License

This project is released under the MIT License.
