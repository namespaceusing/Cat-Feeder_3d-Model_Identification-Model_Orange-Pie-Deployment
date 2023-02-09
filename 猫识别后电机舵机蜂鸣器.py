from machine import Pin,PWM
import time,random
import time,random

#电机
pwm1 = PWM(Pin(26,Pin.OUT))
pwm1.duty(0)

beep = PWM(Pin(25,Pin.OUT))

#舵机
pwm4= PWM(Pin(13,Pin.OUT))
pwm4.freq(50)


#蜂鸣器响三声
beep.freq(660)
for i in range(3):
    beep.duty(10)
    time.sleep(0.5)
    beep.duty(0)    
    time.sleep(0.5)


#电机
pwm1.init()
pwm1.freq(1023)
pwm1.duty(100)
time.sleep(1.8)
pwm1.deinit()

#舵机转动
angle=0
pwm4.duty(int(((angle)/90+0.5)/20*1023))

time.sleep(5)#等待五秒

a=90
pwm4.duty(int(((a)/90+0.5)/20*1023))

time.sleep(1)


beep.freq(660)
beep.duty(30)
time.sleep(2)
beep.duty(0)
time.sleep(1)




