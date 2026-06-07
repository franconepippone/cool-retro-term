
import socket
import json

class TerminalController:
    def __init__(self, host='127.0.0.1', port=55432):
        self.host = host
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def _send(self, payload: str):
        if isinstance(payload, str):
            data = payload.encode('utf-8')
        else:
            data = str(payload).encode('utf-8')
        self.sock.sendto(data, (self.host, self.port))

    def set_profile_from_dict(self, profile: dict):
        # Send full profile object
        self._send(json.dumps({"profile": profile}))

    def set_profile(self, profile_name: str):
        # Send an existing profile name saved inside the terminal
        self._send(json.dumps({"profile": profile_name}))

    def set_settings(self, settings: dict):
        self._send(json.dumps({"settings": settings}))

    def set_jitter(self, value: float):
        self._send(json.dumps({"jitter": value}))

    def set_horizontal_sync(self, value: float):
        self._send(json.dumps({"horizontalSync": value}))

    def set_static_noise(self, value: float):
        self._send(json.dumps({"staticNoise": value}))

    def set_flickering(self, value: float):
        self._send(json.dumps({"flickering": value}))




def simulate_corruption(tc: TerminalController, amount: float):
    tc.set_jitter(amount)
    tc.set_horizontal_sync(amount)
    tc.set_static_noise(amount)
    tc.set_flickering(amount)

if __name__ == '__main__':
    # Quick demo
    print("hello")
    tc = TerminalController()
    tc.set_profile("custom1")
    simulate_corruption(tc, 1)
    import time
    time.sleep(1)