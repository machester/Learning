from http.client import HTTPConnection
from urllib import parse

c = HTTPConnection("www.python.org", 80)
c.request("HEAD", "/index/html")

resp = c.getresponse()

print("status", resp.status)
for name, value in resp.getheaders():
        print(name, value)