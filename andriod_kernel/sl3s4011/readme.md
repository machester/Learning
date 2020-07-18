1. sysfs 生成文件路径

   mtk8167平台，硬件接i2c-2

   ```txt
   /sys/class/i2c-adapter/i2c-2/2-0051/
   ```

2. 生成文件列表

   ```txt
   auto_report 	
   debug     
   report_interval 
   epc_part 
   tid_part  
   usr_part
   ```

   

3. 文件说明

   - auto_report

     自动循环读取 sl3s4011 芯片epc, tid, user 三个分区数据，并打印kernel LOG 信息

   - debug

     开关打印模块的 kernel LOG 信息

   - report_interval 

     自动循环读取的时间间隔

   - epc_part

     读取**EPC**分区信息，从epc分区的第二个寄存器位开始读取

     ```shell
     tb8163p3_bsp:/sys/class/i2c-adapter/i2c-2/2-0051 # cat epc_part
     E200680DAAAA00000000000000000000AAAA
     ```

     

   - tid_part

     读取**TID**分区信息

     ```shell
     tb8163p3_bsp:/sys/class/i2c-adapter/i2c-2/2-0051 # cat tid_part
     E200680D0000400659F3
     ```

   - usr_part

     读取**USR**分区信息，回传数据为前30个寄存器地址信息，总寄存器数为200个

     ```shell
     tb8163p3_bsp:/sys/class/i2c-adapter/i2c-2/2-0051 # cat usr_part
     ABCDEFABCEDFABCDEFABCEFA00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
     ```

     