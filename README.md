# Filenotify

Filenotify is a binary tools for monitoring filesystem modification and store event in influxdb.


## Donwload

You can download binary from this link : [filenotify.tar.gz](https://framagit.org/matgou/filenotify/-/jobs/artifacts/master/raw/filenotify.tar.gz?job=build)

## Installation

Use Make to build and install the binarny

```bash
make && sudo make install
```
## Exemple of usage 

### A Influx/Grafana dashboard of file status

With the filenotify daemon, you can store file's status in influxdb and build Grafana dashboard to visualise file in directory.

![influxgrafanascreenshot](https://framagit.org/matgou/filenotify/-/raw/master/docs/influx_grafana_screenshot.png?inline=false)

## General usage

```bash
bin/filenotify -c filenotify.config -d
```

Filenotify will listen kernel's inotify event to do some stuff when file creating or deleting.
He can store event in influxdb database or log it in a logfile. But he can also lauch a shell script.

```mermaid
graph TB

  SubGraph1 --> SubGraph1Flow
  subgraph "InfluxDB"
  SubGraph1Flow(Http interface)
  SubGraph1Flow --> Database
  end

  subgraph "FileNotify"
  kernel[Inotify Kernel Event] --> daemon[Filenotify daemon]
  daemon --> SubGraph1[Module influxdb]
  daemon --> mlog[Module log]
  mlog --> log[logfile]
  daemon --> mexec[Module exec]
  mexec --> bash[Bash command]
end
```

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License
[GNU](https://www.gnu.org/licenses/licenses.fr.html)
