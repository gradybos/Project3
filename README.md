# Driver's Ed Car System (with Windshield Wipers)
### By Grady Bosanko and Leo Pratt-Thomas

## System Summary
Our system is designed to improve the safety of driver's ed vehicles.
In order to start the car, the driver and passenger seats must be occupied and 
the driver and passenger seatbelts must be secured. If any of these requirements
are not met, an alarm will sound and the user will see a message explaining why 
their ignition attempt failed. If the user fails an ignition attempt, they can press
the ignition button again to disable the alarm and try again. While the engine is
running, the user can press the ignition button to turn off the engine.

Once the engine is running, the windshield wipers of the car are enabled. We have
implemented a dial to switch the wipers between off, interval, low, and high.
The interval setting waits for a set amount of time between wipes, which can be
selected by the user as a short, medium, or long delay. The low and high settings
do not wait between wipes, and function at different speeds. The user can freely toggle
between these states even in the middle of a cycle. Turning the engine off automatically
turns the wipers off.

## Design Alternatives
We had to choose between either using a continuous servo or a positional servo,
and we ended up choosing to use the positional servo. At first we tried to use the
continuous servo to have an easier time implementing multiple speeds, but it turned out
to be nearly impossible to fulfil our requirement of moving the wipers exactly 67 degrees.
Using the positional servo, we were able to achieve a much greater degree of precision.
By incrementing the duty cycle with each system update at different rates, we were also
able to achieve multiple speeds on top of this additional precision. Thus, the positional
servo was clearly the better choice.

## Test Cases

| Specification | Test Result | Comment |
| ------------- | ----------- | ------- |
|Enable engine start (i.e., light the green LED) while both seats are occupied and seatbelts fastened. Otherwise print appropriate error message. | Pass | Appropriate error messages are printed. |
|Start the engine (i.e., light the blue LED, turn off Green) when ignition is enabled (green LED) and ignition button is pressed  (i.e., before the button is released). | Pass | |
| Keep the engine running even if the driver/passenger should unfasten belt and exit the vehicle. | Pass | |
| When the engine is running, stop the engine once the ignition button has been pushed and then released. | Pass | Engine shuts off as button is released consistently |
| User can attempt ignition again after a failed attempt. | Pass | Yes, pressing the ignition button while alarm is going off turns the alarm off and allows another attempt |
| Wiper mode selector toggles between OFF, INT, LOW, and HIGH while the engine is running. | Pass | |
| Interval mode does one "LOW" wipe, then waits for the set interval before the next. | Pass | |
| Selected interval delay is displayed properly | Pass | Interval is displayed as SHORT, MEDIUM, or LONG |
| LOW and HIGH modes work as intended | Pass | No visible delay when wipers reach their peaks and high speed is visibly faster than low speed |
| Selected wiper mode is displayed while engine runs. | Pass | Some flickering is visible on edges of potentiometer thresholds, but correct mode is displayed otherwise |
| Wiper mode is displayed as "OFF" while engine is off. | Pass | |
| Toggling between modes while wipers are mid cycle does not disrupt current wiper cycle. | Pass | The wipers will continue rising or falling until reaching a peak, then will act as they should given their new mode |