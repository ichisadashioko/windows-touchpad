# Touchpad Required HID Top-Level Collections

A Windows Precision Touchpad device shall expose 3 mandatory top-level collections; Windows Precision Touchpad, Mouse and Configuration. An optional (recommended) collection for firmware update can also be implemented.

![](https://docs.microsoft.com/en-us/windows-hardware/design/images/implementationfig4hidcollections.png)

__Figure 1 Windows Precision Touchpad HID Collections__

## Mouse collection

// TODO

## Configuration collection

// TODO

## Windows Precision Touchpad collection

Using the HID protocol in Windows 8.1, a Windows Precision Touchpad shall provide a top-level collection that appears as a digitizer/touchpad (Page 0x0D, Usage 0x05).

The Windows Precision Touchpad collection provides rich multi-contact and button reporting to the host as well as device information that pertains to those reports. The collection shall support two feature reports: one that allows the host to obtain device capabilities, and the other to obtain the device's certification status. The mandatory input report is specified in detail in the following section. An optional (highly recommended) feature report can be implemented to obtain latency mode hints from the host to achieve required power consumption on USB devices in sleep mode.

### Device capabilities feature report

The device capabilities feature report is requested by the host of the Windows Precision Touchpad to elicit the device's contact reporting capabilities and device button type.

The device's contact reporting capability is defined by the maximum number of concurrent surface contacts it may report. A Windows Precision Touchpad shall support a minimum of three concurrent contacts and a maximum of five concurrent contacts and shall report this value by using the specification of the the contact maximum (Page 0x0D, Usage 0x55) in the device capabilities feature reported after the contact count maximum has been reached will be ignored by the host.

The device's button type is defined as either a depressable implementation (also referred to as click-pad type) or a non-depressable implementation (also referred as pressure-pad). Either implementation is acceptable for a Windows Precision Touchpad.

The implementation type shall be specified via the value for button type (Page 0x0D, Usage 0x59) in the device capabilities feature report.

__Table 3 Button Type Usage Values__

| Button Type Value | Implementation |
|---|---|
| 0 | Depressible (Click-pad) |
| 1 | Non-Depressible (Pressure-pad) |

The host can request the device capabilities feature report of a Windows Precision Touchpad at any time after reading the report descriptor.

### Device certification status feature report

The device certification status feature report is requested by the host of the Windows Precision Touchpad to elicit the device's 256-byte blob.

The 256-bytes shall be specified via the vendor specific usage in a vendor defined usage page (Page 0xFF, Usage 0xC5) in the device certification status feature report.

Prior to a device receiving a 256-byte blob attesting to its certification status, it shall implement a default blob as follows:

```
0xfc, 0x28, 0xfe, 0x84, 0x40, 0xcb, 0x9a, 0x87, 0x0d, 0xbe, 0x57, 0x3c, 0xb6, 0x70, 0x09, 0x88, 0x07, 0x97, 0x2d, 0x2b, 0xe3, 0x38, 0x34, 0xb6, 0x6c, 0xed, 0xb0, 0xf7, 0xe5, 0x9c, 0xf6, 0xc2, 0x2e, 0x84, 0x1b, 0xe8, 0xb4, 0x51, 0x78, 0x43, 0x1f, 0x28, 0x4b, 0x7c, 0x2d, 0x53, 0xaf, 0xfc, 0x47, 0x70, 0x1b, 0x59, 0x6f, 0x74, 0x43, 0xc4, 0xf3, 0x47, 0x18, 0x53, 0x1a, 0xa2, 0xa1, 0x71, 0xc7, 0x95, 0x0e, 0x31, 0x55, 0x21, 0xd3, 0xb5, 0x1e, 0xe9, 0x0c, 0xba, 0xec, 0xb8, 0x89, 0x19, 0x3e, 0xb3, 0xaf, 0x75, 0x81, 0x9d, 0x53, 0xb9, 0x41, 0x57, 0xf4, 0x6d, 0x39, 0x25, 0x29, 0x7c, 0x87, 0xd9, 0xb4, 0x98, 0x45, 0x7d, 0xa7, 0x26, 0x9c, 0x65, 0x3b, 0x85, 0x68, 0x89, 0xd7, 0x3b, 0xbd, 0xff, 0x14, 0x67, 0xf2, 0x2b, 0xf0, 0x2a, 0x41, 0x54, 0xf0, 0xfd, 0x2c, 0x66, 0x7c, 0xf8, 0xc0, 0x8f, 0x33, 0x13, 0x03, 0xf1, 0xd3, 0xc1, 0x0b, 0x89, 0xd9, 0x1b, 0x62, 0xcd, 0x51, 0xb7, 0x80, 0xb8, 0xaf, 0x3a, 0x10, 0xc1, 0x8a, 0x5b, 0xe8, 0x8a, 0x56, 0xf0, 0x8c, 0xaa, 0xfa, 0x35, 0xe9, 0x42, 0xc4, 0xd8, 0x55, 0xc3, 0x38, 0xcc, 0x2b, 0x53, 0x5c, 0x69, 0x52, 0xd5, 0xc8, 0x73, 0x02, 0x38, 0x7c, 0x73, 0xb6, 0x41, 0xe7, 0xff, 0x05, 0xd8, 0x2b, 0x79, 0x9a, 0xe2, 0x34, 0x60, 0x8f, 0xa3, 0x32, 0x1f, 0x09, 0x78, 0x62, 0xbc, 0x80, 0xe3, 0x0f, 0xbd, 0x65, 0x20, 0x08, 0x13, 0xc1, 0xe2, 0xee, 0x53, 0x2d, 0x86, 0x7e, 0xa7, 0x5a, 0xc5, 0xd3, 0x7d, 0x98, 0xbe, 0x31, 0x48, 0x1f, 0xfb, 0xda, 0xaf, 0xa2, 0xa8, 0x6a, 0x89, 0xd6, 0xbf, 0xf2, 0xd3, 0x32, 0x2a, 0x9a, 0xe4, 0xcf, 0x17, 0xb7, 0xb8, 0xf4, 0xe1, 0x33, 0x08, 0x24, 0x8b, 0xc4, 0x43, 0xa5, 0xe5, 0x24, 0xc2
```

The host can request the device certification status feature report of a Windows Precision Touchpad at any time after reading the report descriptor.

### Latency mode feature report

The latency mode feature report is sent by the host to a Windows Precision Touchpad to indicate when high latency is desirable for power savings and, conversely, when normal latency is desired for operation. For USB-connected Windows Precision Touchpads, this enables the device to disambiguate between suspended for inactivity (runtime IDLE) and being suspended because the system is entering S3 or Connected Standby.

The latency mode shall be indicated by using the value for the latency mode usage (Page 0x0D, Usage 0x60) in the latency mode feature report.

__Table 4 Latency Mode Usage Values__

| Latency Mode Value | Latency Mode |
|---|---|
| 0 | Normal Latency |
| 1 | High Latency |

### Windows Precision Touchpad input reports

The host makes use of the following usages when extracting contact data from an input report by using the Windows Precision Touchpad collection.

_Table 5 Contact Level Usages_ includes all mandatory usages and supported optional usages that pertain to each unique digitizer contact reported.

__Table 5 Contact Level Usages__

| Member | Description | Page | ID | Mandatory/Optional |
|---|---|---|---|---|
| Contact ID | Uniquely identifies the contact within a given frame | 0x0D | 0x51 | Mandatory |
| X | X coordinate of contact position | 0x01 | 0x30 | Mandatory for T Optional for C |
| Y | Y coordinate of contact position | 0x01 | 0x31 | Mandatory for T Optional for C |
| Tip | Set if the contact is on the surface of the digitizer | 0x0D | 0x42 | Mandatory |
| Confidence | Set when a contact is too large to be a finger | 0x0D | 0x47 | Mandatory |
| Width | Width of contact | 0x0D | 0x48 | Optional |
| Height | Height of contact | 0x0D | 0x49 | Optional |

_Table 6 Report Level Usages_ includes all the mandatory usages that shall be present in all Windows Precision Touchpad input reports.

__Table 6 Report Level Usages__

| Member | Description | Page | ID | Mandatory/Optional |
|---|---|---|---|---|
| Report ID | Windows Precision Touchpad Report ID | 0x0D | 0x05 | Mandatory |
| Scan Time | Relative scan time | 0x0D | 0x56 | Mandatory |
| Contact Count | Total number of contacts to be reported in a given report | 0x0D | 0x54 | Mandatory |
| Button | Indicates Button State | 0x09 | 0x01 | Mandatory |

Any device that does not report all mandatory usages at either the contact or report level will be non-functional as a Windows Precision Touchpad. Mandatory usages are strictly enforced by the Windows host. Where a logical maximum value has not been restricted, it can be optimized to reduce descriptor size.

#### Contact ID

Contact ID uniquely identifies a contact in a report for its lifecycle. The contact ID must remain constant while the contact is detected and reported by the device. Each separate concurrent contact must have a unique identifier. Identifiers can be reused after the previously associated contact is no longer detected or reported. There is no expected numeric range and the values that are used are only limited by the specified logical maximum in the descriptor.

#### X/Y

X and Y report the coordinates of a given contact. A Windows Preicision Touchpad can report two points for each contact.

The first point (known as _T_) is considered the point that the user intended to touch, and is mandatory.

The optional second point (known as _C_) is considered the location of the center of mass of the contact. To report optional height and width usages, reporting the second point is mandatory (and vice-versa).

Devices that are capable of reporting T and C should have a usage array of two X values and two Y values. The values in the first position in the arrays are interpreted as the coordinates for T and the values in the second position are interpreted as the coordinates for C. For devices that opt to report both T and C, the report count for both X and Y usages shall be 2, to indicate the presence of a usage array.

The following global items shall be specified for both X and Y usages:

- Logical minimum & Logical maximum (_ensuring >= 300DPI input resolution_) __Note__ The entire logical coordinate range shall be reportable across both the X and Y axis.
- Physical minimum & Physical maximum (_see Device Integration - Size_)
- Unit & Unit exponent

#### Tip

The tip switch is used to indicate when the contact is on the surface or has left surface of the digitizer. This is indicated by a main item with a report size of 1 bit. When delivering a contact report, the bit should be set when the contact is on the digitizer surface and cleared when the contact has left the surface.

When a contact is being reported with the tip switch clear, the X/Y location that is reported should be the same as the last position that was reported with the tip switch set.

![](https://docs.microsoft.com/en-us/windows-hardware/design/images/implementationfig5twocontactsseparatedlift.jpg)

__Figure 2 Two Contacts with Separated Lift__

With reference to the example in _Figure 2 Two Contacts with Separated Lift_, two contacts are placed on a Windows Precision Touchpad. Some time later, the first contact is lifted while the second remains on the surface. This would be reported as described in _Table 7 Report Sequence for Two Contacts with Separated Lift (Two-Finger Hybird)_.

__Table 7 Report Sequence for Two Contacts with Separated Lift (Two-Finger Hybrid)__

| Report | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 |
|--------|---|---|---|---|---|---|---|---|---|---|---|
| Contact Count | 2 | 2 | 2 |2 | 2 | 2 | 1 | 1 | 1 | 1 | 1 |
| Contact 1 Tip Switch | 1 | 1 | 1 | 1 | 1 | 0 | NR | NR | NR | NR | NR |
| Contact 1 X,Y | X<sub>1</sub>,Y<sub>1</sub> | X<sub>2</sub>,Y<sub>2</sub> | X<sub>3</sub>,Y<sub>3</sub> | X<sub>4</sub>,Y<sub>4</sub> | X<sub>5</sub>,Y<sub>5</sub> | X<sub>5</sub>,Y<sub>5</sub> | NR | NR | NR | NR | NR |
| Contact 2 Tip Switch | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 |
| Contact 2 X,Y | X<sub>1</sub>,Y<sub>1</sub> | X<sub>2</sub>,Y<sub>2</sub> | X<sub>3</sub>,Y<sub>3</sub> | X<sub>3</sub>,Y<sub>4</sub> | X<sub>5</sub>,Y<sub>5</sub> | X<sub>6</sub>,Y<sub>6</sub> | X<sub>7</sub>,Y<sub>7</sub> | X<sub>8</sub>,Y<sub>8</sub> | X<sub>9</sub>,Y<sub>9</sub> | X<sub>10</sub>,Y<sub>10</sub> | X<sub>10</sub>,Y<sub>10</sub> |

#### Confidence

Confidence is used to indicate that the contact does not have any dimensions (height or width) > 25mm that implies that it is not an unintended contact. Windows Precision Touchpads should not reject any contacts in firmware processing, but should forward all contacts to the host and indicate the confidence. After a device has deemed a contact to be unintentional, it shall clear the confidence bit for that contact being reported.

#### Width/Height (optional)

The Width and Height usages represent the width and height of the bounding box around the center of mass of a given contact. The reported values should never be 0 expect when a contact up event is being reported (Tip bit cleared), in which case they shall both be 0. If Height and Width are reported they shall be accurate within +/-2mm of the actual contact dimensions.

The following global items shall be specified for both Width and Height usages:

- Logical minmum & Logical maximum (this is relative to the min/max specified for X/Y)

#### Scan time

Scan Time reports relative digitizer time in 100us units. It represents the delta from the first frame that was reported after a device starts reporting data subsequent to a period of inactivity. The first scan time received is treated as a base time for subsequent reported times. The deltas between reported scan times should reflect the scanning frequency of the digitizer. It is important to note that unlike other usages, the host does not allow any flexibility for the unit for the scan time usage. It must be in 100us units. The value is expected to roll over, as only 2 bytes are allocated to the counter.

The scan time value should be the same for all contacts within a frame.

#### Contact count

Contact count is used to indicate the number of contacts that are being reported in a given frame irrespective of their associated tip switch.

#### Button

The button bit specifies the up/down state of the Windows Precision Touchpad button. Irrespective of button type implementation, when the button has received the required amount of activation force its down state shall be reported by setting the button bit. When the activation force applied to the button falls below the required threshold, the up state shall be reported by clearing the button bit.

![](https://docs.microsoft.com/en-us/windows-hardware/design/images/implementationfig6contactwithbuttondownup.jpg)

__Figure 3 Contact with Button Down and Up__

With reference to the example shown in _Figure 3 Contact with Button Down and Up_, a contact is placed on a Windows Precision Touchpad with sufficient activation force to invoke a button down. Some later time the activation force is reduced so that it invokes a button up, while the contact remains on the surface for some additional time. This would be reported as described in _Table 8 Report Sequence for Contact with Button Down and Up._

__Table 8 Report Sequence for Contact with Button Down and Up__

| Report | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 |
|--------|---|---|---|---|---|---|---|---|---|---|---|
| Contact Count | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
| Button | 1 | 1 | 1 | 1 | 1 | 0 | NR | NR | NR | NR | NR |
| Contact 2 Tip Switch | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 |
| Contact 2 X,Y | X<sub>1</sub>,Y<sub>1</sub> | X<sub>2</sub>,Y<sub>2</sub> | X<sub>3</sub>,Y<sub>3</sub> | X<sub>3</sub>,Y<sub>4</sub> | X<sub>5</sub>,Y<sub>5</sub> | X<sub>6</sub>,Y<sub>6</sub> | X<sub>7</sub>,Y<sub>7</sub> | X<sub>8</sub>,Y<sub>8</sub> | X<sub>9</sub>,Y<sub>9</sub> | X<sub>10</sub>,Y<sub>10</sub> | X<sub>10</sub>,Y<sub>10</sub> |

![](https://docs.microsoft.com/en-us/windows-hardware/design/images/implementationfig7buttononlydownup.jpg)

__Figure 4 Button Only Down and Up__

With reference to the example shown in _Figure 4 Button Only Down and Up_, a non-capacitive contact is placed on a Windows Precision Touchpad with sufficient activation force to invoke a button down. Some later time, the activation force is reduced so that it invokes a button up. This would be reported as described bellow in _Table 9 Report Sequence for Button Only Down and Up_.

__Table 9 Report Sequence for Button Only Down and Up__

| Report | 1 | 2 | 3 | 4 | 5 |
|---|---|---|---|---|---|
| Contact Count | 0 | 0 | 0 | 0 | 0 |
| Button | 1 | 1 | 1 | 1 | 0 |

#### Packet reporting modes
