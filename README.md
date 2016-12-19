# Web of Things: HATEOAS vs. Thing Description

Project from a seminar about WoT at the University of Passau.
Implementation of servers and clients on NodeMCU following the HATEOAS and TD approach for a WoT-interface.

## What is already there:
+ Custom lightweight implementation of a HTTP server.
+ Simple interface with strobe mode for NodeMCUs builtin LED (HATEOAS).
+ Interface for controlling RGB LEDs with strobe mode (HATEOAS).
+ Resolution of equivalent classes/resources via the SPRAQL-endpoint at lov.okfn.org
+ Discovery of new things within the subnet with dumb ping scan.
+ Server for light thing with only red/white colors and strobe (HATEOAS and TD)

## What must still be done:
+ Door bell client controlling speaker (HATEOAS and TD). Therefore discovery of the speaker.
+ All TD servers with different vocabularies.
+ Maybe change SPARQL endpoint to own one, so we can do the mapping of testbeds by our own.
