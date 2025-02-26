# Developer Documentation for Daemonsync

This program is published under the GPLv2 license.

## Build Requirements

- GCC or compatible C compiler
- POSIX-compliant operating system
- Standard C libraries:
  - stdio.h
  - stdlib.h
  - string.h
  - unistd.h
  - signal.h
  - sys/types.h
  - sys/stat.h
  - libgen.h
  - errno.h
  - fcntl.h
  - linux/limits.h

## Build Process

The project includes a simple build script `make.sh` that compiles the program:

```bash
./make.sh
```

The script executes:
```bash
gcc -o daemonsync daemonsync.c
```

To build manually:
```bash
gcc -o daemonsync daemonsync.c -Wall
```

## Project Structure

- `daemonsync.c` - Main source code
- `make.sh` - Build script
- `README.md` - User documentation
- `DEVREADME.md` - Developer documentation

## Development Guidelines

1. Code Style
   - Use consistent indentation (4 spaces)
   - Add comments for complex logic
   - Keep functions focused and manageable

2. Error Handling
   - All system calls should check return values
   - Provide meaningful error messages
   - Clean up resources on error paths

3. Documentation
   - Update README.md when adding/changing features
   - Document any new configuration options
   - Keep example usage current

## Testing

Before committing changes:
1. Build with warnings enabled
2. Test basic functionality:
   - Daemon creation
   - Status checking
   - Configuration management
   - Stopping daemons
3. Test error conditions:
   - Invalid configurations
   - Missing permissions
   - Resource limitations

## Documentation Maintenance

The README.md should be updated when:
- Adding new features
- Changing command-line options
- Modifying file locations
- Updating requirements
- Changing error messages or status codes
- Adding new example use cases

## Runtime Directory

The program uses `~/userrun/` for all runtime files. This directory:
- Is created automatically if missing
- Stores PID, configuration, and log files
- Should be considered in any new feature development

## Future Development Considerations

Potential areas for enhancement:
- Configuration validation
- Signal handling customization
- Logging rotation
- Security improvements
- Resource limits
