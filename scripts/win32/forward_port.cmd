:: usage: forward_port.cmd <local port> <local address>
@echo Forwarding port "0.0.0.0:%~1" to "%~2:80"

@netsh interface portproxy add v4tov4 listenport=%~1 listenaddress=0.0.0.0 connectport=80 connectaddress=%~2
@netsh advfirewall firewall add rule name="Allow EKonyv port forwarding" dir=in action=allow profile=private protocol=TCP localport=%~1

@echo Done.