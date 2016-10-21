# BB_EVS:

This is an example for the usage of the BlueberryE UnoEVS.

It shows how to control the UnoEVS with commands transferred via SPI.
Measurements are triggered from a master by "external" SPI commands. This
master may e.g. be an Uno335. The measurement results are transferred back
the master. All communication between master and UnoEVS is controlled by
the master. The UnoEVS triggers the measurements in the sensors of
the board and performs all necessary calculations to get physical values.
