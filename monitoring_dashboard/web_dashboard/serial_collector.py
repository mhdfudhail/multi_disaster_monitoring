import serial
from time import sleep

def collectData():
    serialData = serial.Serial('COM4', 9600, timeout = 1.0)
    sleep(2)

    serialData.reset_input_buffer()
    print("----collecting----")
    while True:
        sleep(0.01)    
        if serialData.in_waiting >0:
            myData = serialData.readline().decode('utf-8').rstrip()
            # print(myData)
            
            data = myData.split(",")
            if len(data)>0:
                print(data)
                print("----collected----")
                break
    return data



            

if __name__ == '__main__':
    while True:
        value = collectData()
        print(value)
        sleep(2)

