# Daemonsync - Universal Daemon Manager

Daemonsync is a flexible daemon management tool that can be used to run and manage any program as a daemon process. The program uses its own filename to create unique configuration, PID, and log files, allowing you to create multiple daemon managers by simply renaming the executable.

## Features

- Run any program as a daemon
- Monitor daemon status
- Graceful stopping with fallback to force kill
- Configurable command line settings
- Automatic log file management
- Multiple daemon instances through program renaming
- User-specific runtime directory

## Installation

1. Compile the program:
```bash
gcc -o daemonsync daemonsync.c
```

2. (Optional) Create specific daemon managers by copying and renaming:
```bash
cp daemonsync myapp-daemon
cp daemonsync webserver-daemon
```

## File Locations

For a program named `myapp-daemon`, the following files will be used:
- PID file: `~/userrun/myapp-daemon.pid`
- Configuration file: `~/userrun/myapp-daemon.cnf`
- Log file: `~/userrun/myapp-daemon.log`

The `~/userrun/` directory will be automatically created if it doesn't exist.

## Usage

### Basic Commands

```bash
myapp-daemon help     # Show help and file locations
myapp-daemon status   # Check if daemon is running
myapp-daemon run      # Start the daemon
myapp-daemon stop     # Stop the daemon
myapp-daemon getcnf   # Show current configuration
myapp-daemon setcnf <command> [args...]  # Set daemon configuration
```

### Example Usage

1. Create a specific daemon manager:
```bash
cp daemonsync nginx-daemon
```

2. Configure the daemon:
```bash
nginx-daemon setcnf /usr/sbin/nginx -c /etc/nginx/nginx.conf
```

3. Start the daemon:
```bash
nginx-daemon run
```

4. Check status:
```bash
nginx-daemon status
```

5. View configuration:
```bash
nginx-daemon getcnf
```

6. Stop the daemon:
```bash
nginx-daemon stop
```

### Creating Multiple Daemon Managers

You can create multiple daemon managers by copying and renaming the executable. Each instance will maintain its own configuration and state:

```bash
# Create daemon managers for different services
cp daemonsync redis-daemon
cp daemonsync mongodb-daemon
cp daemonsync app-daemon

# Configure each daemon
redis-daemon setcnf /usr/local/bin/redis-server /etc/redis/redis.conf
mongodb-daemon setcnf /usr/bin/mongod --config /etc/mongod.conf
app-daemon setcnf node /path/to/app.js
```

## Output Status

Commands return the following status messages:
- `running`: Daemon is currently running
- `stopped`: Daemon is not running
- `terminated`: Daemon was forcefully terminated (after failed graceful stop)

## Notes

- The daemon runs in the same directory where it was started
- All daemon output is redirected to the log file
- Configuration changes require the daemon to be stopped first
- All files are stored in `~/userrun/` directory
- Stopping a daemon first attempts SIGINT, then falls back to SIGKILL if necessary

## Error Handling

- Returns appropriate error messages for common issues
- Validates configuration before starting daemon
- Ensures clean process termination
- Prevents multiple instances of the same daemon
- Creates runtime directory if it doesn't exist

## Requirements

- Unix-like operating system
- HOME environment variable must be set

## License

This program is published under the GPLv2 license.

## Author

D Brink <danie+daemonsync@brinkworx.net>