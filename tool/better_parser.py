import pyorb

with open('D:/tmp/rtos-trace/task_notify.trace', 'rb') as trace:
    orb = pyorb.Orb(
        source=pyorb.orb_source_io(trace),
        withTPIU=False,
        forceSync=False
    )

    while True:
        packet = orb.rx()
        if packet is None:
            break

        if isinstance(packet, pyorb.Empty):
            continue

        if isinstance(packet, pyorb.swMsg):
            print(packet.ts, 'MSG', packet.srcAddr, hex(packet.value))

        if isinstance(packet, pyorb.TSMsg):
            print(packet.ts, 'TS', packet.timeStatus, packet.timeInc)

        # print(packet)

print('Finished')