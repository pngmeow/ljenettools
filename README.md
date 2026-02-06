# ljenettools
- Bsendpacket
- Host_AccumulateTime hook

## Example

```lua
enable_timescale(true) -- Manipulates Host_AccumulateTime
set_timescale(0.5) -- Sets the speed of Host_AccumulateTime

send_packet(false) -- Writes false to bSendPacket
print(choked_commands()) -- Returns the amount of choked packets by bSendPacket

send_packet(true) -- Writes true to bSendPacket
```

special credits to:
- rva for teaching me how to do this

- kNopeson for the uc post
