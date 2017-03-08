# UdpHoleServer
UDP打洞服务器代码 UDP NAT traval Server Code

# 工作流程
被打洞端上报自己的局域网IP和设备标识(MD5)</br>
请求打洞端也需要上报自己的局域网IP并告诉服务器需要打洞的设备标识(MD5)</br>
服务器找到匹配的设备标识后,分别向两方发送另一方的公网IP和局域网IP</br>
