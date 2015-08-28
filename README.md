# Islandora Jpylyzer

## Introduction

Provides a Drush script to verify repository JP2s using Jpylyzer.

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/islandora/islandora)
* [Jpylyzer](http://jpylyzer.openpreservation.org/) should be installed as a binary executable and able to be run by the webserver.

## Installation

For the module itself, install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

For Jpylyzer, a .deb file is provided at the site; this can be installed on Debian-based machines, typically using:
```sh
sudo dpkg -i installer.deb
```

## Configuration

Drush options can be used to narrow the JP2 set to verify. Use `drush help verify-jp2s` for more information.

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## Maintainers/Sponsors

Current maintainers:

* [Daniel Aitken](https://github.com/qadan)

## Development

If you would like to contribute to this module, please check out our helpful [Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers) info, as well as our [Developers](http://islandora.ca/developers) section on the Islandora.ca site.

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
