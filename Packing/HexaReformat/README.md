# HexaReformat

Small and independent program for translating the Boost.Serialization of hexactrl-sw
into the specific input format for this Packing module of ldmx-sw.

## Building

This program is meant to be independent of ldmx-sw in order to ensure
that no ldmx-sw tools are being indirectly used. With this in mind,
it is compiled separately from ldmx-sw even though it is housed here.

#### Requirements

- cmake
  - For configuring the build
- Boost.Serialization
  - For decoding the archive produced by hexactrl-sw
- Boost.ProgramOptions
  - For inputing command-line arguments to the executable
- ROOT
  - For creating the translated RawEventFile to be input into ldmx-sw

The container already has these tools, so one can still use the container
for developing and using this program.

#### Start-Up

I have been using the container, so I will leave the container-prefix `ldmx`
in these commands. Ommitting this prefix will give you a good start for
building this software outside the container.

```
cd HexaReformat
ldmx cmake -B build -S . -DCMAKE_INSTALL_PREFIX=${LDMX_BASE}/ldmx-sw/install
ldmx cmake --build build --target install
```

Notice that I can choose where to install the software and I have chosen
to install it _alongside_ ldmx-sw just for convenience.
