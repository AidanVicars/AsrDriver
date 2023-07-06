# AsrDriver

## Description
A usermode program which abuses the AsrDrv106.sys to have arbritrary writes/reads CRs, MSRs, and physical memory.

## Write-up
### Exhibit A 
![alt text](https://github.com/AidanVicars/AsrDriver/blob/master/assets/AsrDrv106Disasm1.PNG?raw=true)

The entry of the DeviceControl routine where the decryption routine is called
