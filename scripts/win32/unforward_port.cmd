:: usage: unforward_port.cmd <local port>
@echo Clearing port forwarding for "0.0.0.0:%~1"

@netsh interface portproxy delete v4tov4 listenaddress=0.0.0.0 listenport=%~1
@netsh advfirewall firewall delete rule name="Allow EKonyv port forwarding" dir=in profile=private protocol=TCP

@echo Done.