# HW 1: DRAM Refresh and Combinational Logic

## 1. DRAM Refresh

1 ExaByte = 10^18 bytes
8 KiloByte = 8\*10^3 bytes

- **(a)** How many DRAM rows does the memory system have?
- *Answer:* 10^18 / (8\*10^3) = 125\*10^12 rows.
- **(b)** How many DRAM refreshes happen in 64ms?
- *Answer:* 64\*10^3 bits per row gets refreshed once every 64ms. 8\*10^18 bits gets refreshed in total once every 64ms.
- **(c)** What is the power consumption of DRAM refresh?
- *Answer:*
- **(d)** What is the total energy consumption of DRAM refresh during a refresh cycle? And during a day?
- *Answer:*

## 2. Main Memory Potpourri

- **(a)** The machine's designer runs two applications A and B (each run alone) on the machine. Although applications A and B have a similar number of memory requests, application A spends a surprisingly larger fraction of cycles stalling for memory than application B does? What might be the reasons for this?
- *Answer:* The application A does not exploit cache locality or it does random accesses to the memory resulting with row-buffer misses.
- **(b)** Application A also consumes a much larger amount of memory energy than application B does. What might be the reasons for this?
- *Answer:* Stalling can be the reason since each time more circuitry gets activated due to row-buffer misses.
- **(c)** When applications A and B are run together on the machine, application A's performance degrades significantly, while application B's performance does not degrade as much. Why might this happen?
- *Answer:* B can benefit from row-buffer hits while A cannot and while run together individual requests from both applications get scheduled which can result with priorizing accesses with row-buffer hits.
- **(d)** The designer decides to use a marter policy to refresh the memory. A row is refreshed only if it has not been accessed in the past 64 ms. Do you think this is a good idea? Why or why not?
- *Answer:* It can be a good idea because of omitting energy-wasting row-refreshes. However, this can work with the assumption that it is guaranteed that charge(a bit) in any capacitor won't change during 64 ms. Better idea would be refreshing each row according to their individual characteristics of keeping a bit as it is.
- **(e)** When this new refresh policy is applied, the refresh energy consumption drops significantly during a run of application B. In contrast, during a run of application A, the refresh energy consumption reduces only slightly. Is this possible? Why or why not?
- *Answer:* B might request larger partition of memory while A might request smaller partition frequently (potentially randomly so it has lots of row-buffer misses). Therefore, energy consumption does not drop as much since bigger partition of memory gets refreshed once every 64ms as before (the policy does not do much in this case).

## 3. Transistor-Level Circuit Design

- Exclusive OR GATE (XOR)

|	in1	|	in2	|	out	|
|:-----:|:-----:|:-----:|
|	0	|	0	|	0	|
|	0	|	1	|	1	|
|	1	|	0	|	1	|
|	1	|	1	|	0	|


- Exclusive NOT OR Gate (XNOR)

## 4. Multiplexer (MUX)

8:1 MUX by using

- AND, OR, NOT gates (as few as possible)
- 2:1 MUXes (as few as possible)

## 5. Logical Completeness

A AND B = (A NOR A) NOR (B NOR B)
A OR B	= (A NOR B) NOR (A NOR B)
NOT A	= A NOR A

## 6. Boolean Logic and Truth Tables

- **(a)**	
	0000 1
	0001 1
	0010 1
	0011 1
	0100 1
	0101 1
	0110 1
	0111 0
	1000 1
	1001 1
	1010 1
	1011 1
	1100 1
	1101 1
	1110 0
	1111 0 
- *Answer:* X = (NOT A3 + NOT A2 + NOT A1) (NOT A2 + NOT A1 + NOT A0)
- **(b)**
	0000 0
	0001 0
	0010 0
	0011 0
	0100 0
	0101 1
	0110 0
	0111 0
	1000 0
	1001 0
	1010 1
	1011 0
	1100 0
	1101 0
	1110 0
	1111 0
- *Answer:* Y = (NOT A3 * A2 * NOT A1 * A0) + (A3 * NOT A2 * A1 * NOT A0)
- **(c)**
- *Answer:* Y = (A3 XOR A2) AND (A2 XOR A1) AND (A1 XOR A0)

