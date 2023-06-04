from machine import Pin, SPI
from fonts import fonts_vga1_16x32
from fonts import fonts_vga2_8x8
from time import sleep
import st7789
import dht
import _thread
import socket
import network
import gc
gc.collect()

ssid = 'PICOPAD'  # Set access point name
password = '12345678'  # Set your access point password

# SPI(1) - display pins
st7789_res = Pin(20, mode=Pin.OUT)
st7789_dc = Pin(17, mode=Pin.OUT)
st7789_cs = Pin(21, mode=Pin.OUT)
st7789_blk = Pin(16, mode=Pin.OUT)

# Display initialization
spi = SPI(0, 62500000, sck=Pin(18), mosi=Pin(19), polarity=1, phase=1)
display = st7789.ST7789(spi, 320, 240, reset=st7789_res, dc=st7789_dc, cs=st7789_cs, backlight=st7789_blk, rotation=1)
display.fill(0)
display.text(fonts_vga1_16x32, "MakerFaire", 20, 40, 0xF800, 0)
display.text(fonts_vga2_8x8, "DHT22 DEMO", 20, 75, 0x0FFF, 0)

# DHT22 sensor initialization on pin 26
dht_sensor = dht.DHT22(Pin(26))

ap = network.WLAN(network.AP_IF)
ap.config(essid=ssid, password=password)
ap.active(True)

temperature = 0
humidity = 0

while not ap.active():
    pass

print(ap.ifconfig())

def generate_response():
    global temperature
    global humidity
    data = f"{temperature}|{humidity}"
    return data

def webserver():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind(('', 80))
    s.listen(5)
    while True:
        conn, addr = s.accept()
        print('Got a connection from %s' % str(addr))
        request = conn.recv(1024)
        print('Content = %s' % str(request))
        response = generate_response()
        conn.send('HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n')
        conn.send(response)
        conn.close()

def measure():
    global temperature
    global humidity
    while True:
        dht_sensor.measure()
        temperature = dht_sensor.temperature()
        humidity = dht_sensor.humidity()
        display.text(fonts_vga1_16x32, "Teplota: {:.2f}C".format(temperature), 20, 90, 0xffff, 0)
        display.text(fonts_vga1_16x32, "Vlhkost: {:.2f}%".format(humidity), 20, 120, 0xffff, 0)
        sleep(2)

_thread.start_new_thread(measure, ())
webserver()
