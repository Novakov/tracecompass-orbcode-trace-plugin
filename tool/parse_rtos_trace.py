import argparse
import inspect

import logging
from functools import wraps
from pprint import pprint

EVENT_ID_SHIFT = 28
EVENT_ID_MASK = 0b1111
CYCCNT_MASK = 0x0FFF_FFFF

EVENT_TASK_SWITCHED_IN = 1
EVENT_TASK_SWITCHED_OUT = 2
EVENT_TASK_CREATED = 3
EVENT_BINARY_SEM_CREATED = 4
EVENT_BINARY_SEM_LOCKING = 5
EVENT_MUTEX_CREATED = 6
EVENT_MUTEX_LOCKING = 7
EVENT_QUEUE_CREATED = 8
EVENT_QUEUE_PUSH_POP = 9
EVENT_TASK_READIED = 10
EVENT_COUNTING_SEM_CREATED = 11
EVENT_COUNTING_SEM_GIVE_TAKE = 12


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('input', type=argparse.FileType('r'))
    parser.add_argument('output', type=argparse.FileType('w'))
    return parser.parse_args()


def event_handler(event_id: int):
    def wrapper(f):
        f.event_id = event_id

        @wraps(f)
        def wrapped(*args, **kwargs):
            result = f(*args, **kwargs)

            if not isinstance(result, tuple):
                return f.__name__, result
            elif len(result) == 1:
                return f.__name__, *result
            else:
                return result

        return wrapped

    return wrapper


class Collector:
    def __init__(self):
        self.current_tcb = 0
        self.task_names = {}

    def resolve_task_name(self, task_tcb: int) -> int:
        return self.task_names.get(task_tcb, f'0x{task_tcb:08X}')

    @event_handler(EVENT_TASK_SWITCHED_IN)
    def task_switched_in(self, tcb):
        self.current_tcb = tcb

        return {
            'TCB': f'0x{tcb:08X}',
            'TaskName': self.resolve_task_name(tcb)
        }

    @event_handler(EVENT_TASK_SWITCHED_OUT)
    def task_switched_out(self, tcb, switch_reason_data: int, blocked_on_object: int, flags: int):
        self.current_tcb = 0
        still_ready = (flags & (1 << 0)) == (1 << 0)
        switch_reason = (switch_reason_data >> 28) & 0b1111

        out_state = 'Blocked'
        blocked_on = ''
        block_operation = ''

        if still_ready:
            out_state = 'Ready'
            blocked_on_object = None
        elif switch_reason == 0:
            out_state = 'Ready'
            blocked_on_object = None
        elif switch_reason == 1:
            out_state = 'Delayed'
            blocked_on_object = None
        elif switch_reason == 2:
            out_state = 'Blocked'
            blocked_on = 'Mutex'
        elif switch_reason == 3:
            out_state = 'Blocked'
            blocked_on = 'Queue'
            block_operation = 'Push'
        elif switch_reason == 4:
            out_state = 'Blocked'
            blocked_on = 'Queue'
            block_operation = 'Pop'
        elif switch_reason == 5:
            out_state = 'Blocked'
            blocked_on = 'BinarySemaphore'
        elif switch_reason == 6:
            out_state = 'Blocked'
            blocked_on = 'EventGroup'
        elif switch_reason == 7:
            out_state = 'Blocked'
            blocked_on = 'CountingSemaphore'
            block_operation = 'Give'
        elif switch_reason == 8:
            out_state = 'Blocked'
            blocked_on = 'CountingSemaphore'
            block_operation = 'Take'
        elif switch_reason == 9:
            out_state = 'Blocked'
            blocked_on = 'CountingSemaphore'
            block_operation = 'Give'
        elif switch_reason == 0xF:
            out_state = 'Blocked'
            blocked_on = 'Other'
        else:
            print(f'Unknown switch out reason {switch_reason}')

        return {
            'TCB': f'0x{tcb:08X}',
            'TaskName': self.resolve_task_name(tcb),
            'OutState': out_state,
            'BlockedOn': blocked_on,
            'BlockingOperation': block_operation,
            'BlockedOnObject': f'0x{blocked_on_object:08X}' if blocked_on_object is not None else '',
        }

    @event_handler(EVENT_TASK_READIED)
    def task_readied(self, tcb: int):
        return {
            'TCB': f'0x{tcb:08X}',
            'TaskName': self.resolve_task_name(tcb)
        }

    @event_handler(EVENT_BINARY_SEM_CREATED)
    def binary_semaphore_created(self, queue: int):
        return {'Semaphore': f'0x{queue:08X}'}

    @event_handler(EVENT_COUNTING_SEM_CREATED)
    def counting_semaphore_created(self, semaphore: int, max_count: int, initial_count: int):
        return {'Semaphore': f'0x{semaphore:08X}', 'MaxCount': max_count, 'InitialCount': initial_count}

    @event_handler(EVENT_COUNTING_SEM_GIVE_TAKE)
    def counting_semaphore_give_take(self, queue: int, flags: int):
        is_success = (flags & (1 << 2)) == (1 << 2)
        is_isr = (flags & (1 << 1)) == (1 << 1)
        is_take = (flags & (1 << 0)) == (1 << 0)
        updated_items_count = (flags >> 16) & 0xFFFF

        if is_isr:
            tcb = 0
        else:
            tcb = self.current_tcb

        data = {
            'Semaphore': f'0x{queue:08X}',
            'TCB': f'0x{tcb:08X}',
            'TaskName': self.resolve_task_name(tcb),
            'ISR': is_isr,
            'UpdatedCount': updated_items_count,
        }

        if is_success:
            if is_take:
                return 'counting_semaphore_taken', data
            else:
                return 'counting_semaphore_given', data
        else:
            if is_take:
                return 'counting_semaphore_take_failed', data
            else:
                return 'counting_semaphore_give_failed', data

    @event_handler(EVENT_MUTEX_CREATED)
    def mutex_created(self, mutex: int):
        return {'Mutex': f'0x{mutex:08X}'}

    @event_handler(EVENT_BINARY_SEM_LOCKING)
    def binary_semaphore_locking(self, queue: int, flags: int):
        is_success = (flags & (1 << 2)) == (1 << 2)
        is_isr = (flags & (1 << 1)) == (1 << 1)
        is_lock = (flags & (1 << 0)) == (1 << 0)

        if is_isr:
            tcb = 0
        else:
            tcb = self.current_tcb

        data = {
            'Semaphore': f'0x{queue:08X}',
            'TCB': f'0x{tcb:08X}',
            'TaskName': self.resolve_task_name(tcb),
            'ISR': is_isr
        }

        if is_success:
            if is_lock:
                return 'binary_semaphore_locked', data
            else:
                return 'binary_semaphore_unlocked', data
        else:
            if is_lock:
                return 'binary_semaphore_lock_failed', data
            else:
                return 'binary_semaphore_unlock_failed', data

    @event_handler(EVENT_MUTEX_LOCKING)
    def mutex_locking(self, mutex: int, flags: int):
        is_success = (flags & (1 << 2)) == (1 << 2)
        is_isr = (flags & (1 << 1)) == (1 << 1)
        is_lock = (flags & (1 << 0)) == (1 << 0)

        if is_isr:
            tcb = 0
        else:
            tcb = self.current_tcb

        data = {
            'Mutex': f'0x{mutex:08X}',
            'TCB': f'0x{tcb:08X}',
            'TaskName': self.resolve_task_name(tcb),
            'ISR': is_isr
        }

        if is_success:
            if is_lock:
                return 'mutex_locked', data
            else:
                return 'mutex_unlocked', data
        else:
            if is_lock:
                return 'mutex_lock_failed', data
            else:
                return 'mutex_unlock_failed', data

    @event_handler(EVENT_QUEUE_CREATED)
    def queue_created(self, queue: int, capacity: int):
        return {'Queue': f'0x{queue:08X}', 'Capacity': capacity}

    @event_handler(EVENT_QUEUE_PUSH_POP)
    def queue_push_pop(self, queue: int, flags: int):
        is_success = (flags & (1 << 2)) == (1 << 2)
        is_isr = (flags & (1 << 1)) == (1 << 1)
        is_push = (flags & (1 << 0)) == (0 << 0)
        is_pop = not is_push
        updated_items_count = (flags >> 16) & 0xFFFF

        if is_isr:
            tcb = 0
        else:
            tcb = self.current_tcb

        data = {
            'Queue': f'0x{queue:08X}',
            'TCB': f'0x{tcb:08X}',
            'TaskName': self.resolve_task_name(tcb),
            'ISR': is_isr,
            'UpdatedItemsCount': updated_items_count
        }

        if is_success:
            if is_push:
                return 'queue_pushed', data
            else:
                return 'queue_popped', data
        else:
            if is_push:
                return 'queue_pushed_failed', data
            else:
                return 'queue_popped_failed', data

    def event_map(self):
        result = {}
        for name, member in inspect.getmembers(self):
            if not inspect.ismethod(member):
                continue
            if not hasattr(member, 'event_id'):
                continue
            sig = inspect.signature(member)
            result[member.event_id] = (member, len(sig.parameters))

        return result


def main(args):
    lines = iter(args.input.readlines())

    line_counter = 0

    def read_line() -> bytes:
        nonlocal line_counter
        line_counter += 1
        line = next(lines).strip()

        assert line != '0xDEADBEEF'
        assert line != '0xCAFEBABE'

        line_len = len(line)
        if (line_len % 2) == 1:
            line_len += 1

        msg = int(line, 16).to_bytes(byteorder='little', length=(line_len - 2) // 2)
        return msg

    def read_first_line_from_packet() -> bytes:
        nonlocal line_counter

        line = ''
        while line != '0xDEADBEEF':
            line_counter += 1
            line = next(lines).strip()

        return read_line()

    def read_last_line_from_packet() -> None:
        nonlocal line_counter

        line_counter += 1
        line = next(lines).strip()
        tail = []
        while line != '0xCAFEBABE':
            tail.append(line)
            line_counter += 1
            line = next(lines).strip()

        if tail:
            print(f'TAIL: {tail} (line_counter = {line_counter})')

    collector = Collector()

    event_map = collector.event_map()

    def do_handle_messages():
        cumulative_timestamp = 0
        last_timestamp = 0

        while True:
            msg = read_first_line_from_packet()

            num = int.from_bytes(msg, byteorder='little')
            event_id = (num >> EVENT_ID_SHIFT) & EVENT_ID_MASK
            cycle_counter = (num & CYCCNT_MASK)

            if cycle_counter >= last_timestamp:
                cumulative_timestamp += cycle_counter - last_timestamp
            else:
                cumulative_timestamp += CYCCNT_MASK - last_timestamp + cycle_counter

            last_timestamp = cycle_counter

            timestamp_s = cumulative_timestamp / 50e6
            # print(f'ts_s: {timestamp_s:07.3f} cycle_counter = {cycle_counter} (line = {line_counter})')
            # if timestamp_s >= 246:
            #     print(line_counter)
            #     break

            if event_id in event_map:
                handler, additional_count = event_map.get(event_id)
                additional = [int.from_bytes(read_line(), byteorder='little') for i in range(0, additional_count)]
                event_type, event_data = handler(*additional)
                # print(f'{event_type} {event_data}')
                args.output.write(f'Timestamp: {timestamp_s:.6f} Event type: {event_type}\n')
                for k, v in event_data.items():
                    args.output.write(f"    {k}: '{v}'\n")
            elif event_id == EVENT_TASK_CREATED:
                tcb = int.from_bytes(read_line(), byteorder='little')
                name_length = int.from_bytes(read_line(), byteorder='little')
                name = bytearray()
                while len(name) < name_length:
                    name += read_line().strip(b'\0')
                name = name.strip(b'\0').decode('utf-8')
                # collector.on_task_created(cycle_counter=cycle_counter, tcb=tcb, name=name)
                args.output.write(f'Timestamp: {timestamp_s:.6f} Task created TCB: 0x{tcb:08X} Name: {name}\n')

                print(f'TCB: 0x{tcb:08X} Name: {name}')

                if name not in collector.task_names.values():
                    collector.task_names[tcb] = name
                else:
                    for i in range(1, 128):
                        modified_name = name + str(i)
                        if modified_name not in collector.task_names.values():
                            collector.task_names[tcb] = modified_name
                            break

            else:
                logging.warning(f'Unrecognized message 0x{num:08X} (event_id: {event_id})')

            read_last_line_from_packet()

    try:
        do_handle_messages()
    except StopIteration:
        pass

    # pprint(collector.task_names)


main(parse_args())
