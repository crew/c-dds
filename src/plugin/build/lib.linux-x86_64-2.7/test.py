import dds
import signal
import os

class Caller:
    ident = 0124
    def getName(self):
        return "Caller"

    def call(self):
        dds.send({"Hello":"World"})

a = Caller()
def do_nothing(*args):
    pass
a.call()
signal.signal(signal.SIGUSR1, do_nothing)
os.kill(os.getpid(), signal.SIGUSR1)
