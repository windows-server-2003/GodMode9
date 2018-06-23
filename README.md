# GodMode9 - MultiThreadMod
A multi-thread support variant of [GodMode9](https://github.com/d0k3/GodMode9) 

## What you can do with this
Browse dirs while copying/injecting/etc.!
Browse while a script is running!
Disable/Enable these feauture(you can't do while a background task is running though)

## What you can't do
Process multiple file operations.
Run multiple scripts.
Run a script while a file operation is running

## File operation speed
At the initial release, operation speed was f..king slow and useless, but it's improved now.
Here is the proof :
Seq. 85.1MB SD(0:) -> RAMDRIVE(9:)
origin(1.6.0) : 15.38s
MTmod v1.1    : 16.57s
MTmod v1.0    : 24.39s
MTmod v0.0    : 137s <- wtf

## Warning
__This is powerful stuff__, it provides you with the means to do basically any thinkable modification to any system data available on the 3DS console. However, precautions are taken so you don't accidentally damage the data of your console. The write permissions system protects you by providing warnings and forces you to enter an unlock sequence for enabling write permissions. It is not possible to overwrite or modify any important stuff without such unlock sequences and it is not possible to accidentally unlock something.

__As always, be smart, keep backups, just to be safe__.

## How to build this / developer info
Build `GodMode9.firm` via `make firm`. This requires [firmtool](https://github.com/TuxSH/firmtool), [Python 3.5+](https://www.python.org/downloads/) and [devkitARM](https://sourceforge.net/projects/devkitpro/) installed).

You may run `make release` to get a nice, release-ready package of all required files. To build __SafeMode9__ (a bricksafe variant of GodMode9, with limited write permissions) instead of GodMode9, compile with `make FLAVOR=SafeMode9`. To switch screens, compile with `make SWITCH_SCREENS=1`. For additional customization, you may choose the internal font by replacing `font_default.pbm` inside the `data` directory. You may also hardcode the brightness via `make FIXED_BRIGHTNESS=x`, whereas `x` is a value between 0...15.

Further customization is possible by hardcoding `aeskeydb.bin` (just put the file into the `data` folder when compiling). All files put into the `data` folder will turn up in the `V:` drive, but keep in mind there's a hard 3MB limit for all files inside, including overhead. A standalone script runner is compiled by providing `autorun.gm9` (again, in the `data` folder) and building with `make SCRIPT_RUNNER=1`.

To build a .firm signed with SPI boot keys (for ntrboot and the like), run `make NTRBOOT=1`. You may need to rename the output files if the ntrboot installer you use uses hardcoded filenames. Some features such as boot9 / boot11 access are not currently available from the ntrboot environment.

## License
You may use this under the terms of the GNU General Public License GPL v2 or under the terms of any later revisions of the GPL. Refer to the provided `LICENSE.txt` file for further information.

## Credits
This mod could not be made without original [GodMode9](https://gitub.com/d0k3/GodMode9). Thanks for all contribution to it.
* **d0k3**, for developing original GodMode9
* **vonanzard**, for providing nice ideas of the splash
* **Archshift**, for providing the base project infrastructure
* **Normmatt**, for sdmmc.c / sdmmc.h and gamecart code, and for being of great help on countless other occasions
* **Cha(N)**, **Kane49**, and all other FatFS contributors for [FatFS](http://elm-chan.org/fsw/ff/00index_e.html)
* **Wolfvak** for ARM11 code, FIRM binary launcher, exception handlers, PCX code, Makefile and for help on countless other occasions
* **SciresM** for helping me figure out RomFS and for boot9strap
* **SciresM**, **Myria**, **Normmatt**, **TuxSH** and **hedgeberg** for figuring out sighax and giving us access to bootrom
* **ihaveamac** for first developing the simple CIA generation method and for being of great help in porting it
* **b1l1s** for helping me figure out A9LH compatibility
* **Gelex** and **AuroraWright** for helping me figure out various things
* **stuckpixel** for the new 6x10 font and help on various things
* **Al3x_10m** for help with countless hours of testing and useful advice
* **WinterMute** for helping me with his vast knowledge on everything gamecart related
* **profi200** for always useful advice and helpful hints on various things
* **windows-server-2003** for the initial implementation of if-else-goto in .gm9 scripts
* **Kazuma77** for pushing forward scripting, for testing and for always useful advice
* **JaySea**, **YodaDaCoda**, **liomajor**, **Supster131**, **imanoob**, **Kasher_CS** and countless others from freenode #Cakey and the GBAtemp forums for testing, feedback and helpful hints
* **Shadowhand** for being awesome and [hosting my nightlies](https://d0k3.secretalgorithm.com/)
* **Plailect** for putting his trust in my tools and recommending this in [The Guide](https://3ds.guide/)
* **SirNapkin1334** for testing, bug reports and for hosting the official [GodMode9 Discord channel](https://discord.gg/EGu6Qxw)
* **Project Nayuki** for [qrcodegen](https://github.com/nayuki/QR-Code-generator)
* **Amazingmax fonts** for the Amazdoom font
* The fine folks on **freenode #Cakey**
* All **[3dbrew.org](https://www.3dbrew.org/wiki/Main_Page) editors**
* Everyone I possibly forgot, if you think you deserve to be mentioned, just contact me!
