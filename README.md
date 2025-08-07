# NexOS

## Keys:
### Modifiers

Modifiers are sent as a uint8 along with every keypress.

> 0b1010 0101

| bit value |  meaning   |
|-----------|------------|
|     1     | shift held |
|     2     | ctrl held  |
|     4     |  alt held  |
|     8     |  mod held  |
|    16     |  capslock  |
|    32     |  num lock  |
|    64     |  scrl lock |
|    128    |  None yet  |