watchasayin
===========

WARNING: Current project in git does not work (will be fixed asap).

A tool for encrypting Skype calls (Windows only).

The central Skype CA as the root-of-trust is replaced by a direct OTR based AKE in which each party verifies the other's fingerprint.
The exchanged key is used to establish an additional layer of encryption on top of Skype's own. In essence ws2_32.dll socket functions are hooked and outgoing/incoming data belonging to a to be protected call is encrypted/decrypted.
Due to this approach at the current state of development only calls made over direct p2p connections are supported (in essence no relayed calls).

Uses libotr, libgcrypt, N-CodeHook and Skype4COM. The first two were customly cross-compiled a while ago and are thus a bit outdated (did not check so far which security implications arise from this).

More info to follow.

No guarantes, warantees or whatsoever. Use as is or don't.
