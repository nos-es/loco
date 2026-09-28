# loco


```text
$ ./loco ~/Downloads/proxmox-ve_9.2-1.iso.torrent

▄▄
██
██ ▄███▄ ▄████ ▄███▄
██ ██ ██ ██    ██ ██
██ ▀███▀ ▀████ ▀███▀

Torrent: proxmox-ve_9.2-1.iso
Peers: 33

Connected to Peer: 94.32.93.23:50413
Downloading: piece 837 / 3255 (25.7%)
```

`loco` is a command line BitTorrent client that I built to understand how BitTorrent works at a lower level.

The goal was not to build a replacement for clients like qBittorrent or Transmission. I wanted to keep the scope small enough that I could implement and understand the important parts of a BitTorrent download myself.

## Features

- BitTorrent v1
- local `.torrent` files
- single-file torrents
- custom Bencode parser
- torrent metadata parsing
- SHA-1 `info_hash` calculation
- HTTP and HTTPS trackers using libcurl
- compact and non-compact tracker peer lists
- IPv4 peers
- TCP peer connections
- BitTorrent handshake
- basic Peer Wire Protocol messages
- bitfield and `have` handling
- choking and unchoking
- block requests and piece messages
- sequential piece downloading
- SHA-1 verification of downloaded pieces
- retrying pieces after peer failures
- switching to another peer when necessary
- writing pieces to the correct file offset
- simple terminal download progress

## Scope

`loco` intentionally supports only a small part of BitTorrent.

Version 0.1 does **not** support:

- Magnet links
- DHT
- UDP trackers
- BitTorrent v2 or hybrid torrents
- multi-file torrents
- uTP
- Peer Exchange
- incoming peer connections
- encrypted peer connections
- parallel downloads from multiple peers
- resume support
- file priorities
- uploading or seeding

Only one peer is used at a time. If the current peer cannot provide the required piece or the connection fails, `loco` tries another peer returned by the tracker.

## Build

The project uses `make`.

```sh
make
```

For a build with AddressSanitizer and UndefinedBehaviorSanitizer:

```sh
make sanitize
```

## Usage

```sh
./loco <torrent-file>
```

Example:

```sh
./loco ~/Downloads/debian-13.7.0-amd64-netinst.iso.torrent
```

The downloaded file is written to the current working directory.

## Tests

Run the test suite with:

```sh
make test
```

The project also contains sanitizer builds for finding memory errors and undefined behavior during development.

## Why I built this

I have always found BitTorrent interesting and wanted to understand what is actually happening behind a normal torrent download.

Instead of only reading about the protocol, I wanted to build a small client myself: parse the torrent file, contact a tracker, connect to real peers, exchange protocol messages and eventually receive and verify the actual file data.

At the same time, I was learning C. This project ended up being a very good way for me to get more comfortable with the language because I had to use it for something real instead of only solving small isolated exercises.

Working with binary data, pointers, memory management, sockets and network byte order made me understand C much better than I did before starting this project.

The project was sometimes frustrating and took quite a while to finish, but I learned a lot from it. I also ended up enjoying C much more than I expected.

Keeping the scope limited was important to me. BitTorrent has many more features and extensions, but for this project I mainly wanted to understand the path from a `.torrent` file to a correctly downloaded and verified file.
