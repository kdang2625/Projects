# import asyncio
# from bleak import BleakClient, BleakScanner

# SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
# CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"

# def notification_handler(sender, data):
#     """Callback for handling incoming notifications."""
#     try:
#         decoded = data.decode('utf-8')
#     except UnicodeDecodeError:
#         decoded = data.hex()
#     print(f"\nNotification from {sender}: {decoded}")

# async def get_user_input(prompt: str) -> str:
#     """Asynchronously get user input without blocking the event loop."""
#     return await asyncio.to_thread(input, prompt)

# async def run():
#     print("Scanning for BLE devices...")
#     devices = await BleakScanner.discover()
#     target = None

#     # Find the target device by name (adjust "SkateABS" as needed)
#     for d in devices:
#         print(f"Found device: {d.name}, {d.address}")
#         if d.name and "SkateABS" in d.name:
#             target = d
#             break

#     if target is None:
#         print("Target device not found. Ensure it is advertising!")
#         return

#     print(f"Connecting to {target.address}...")
#     async with BleakClient(target.address) as client:
#         if not client.is_connected:
#             print("Failed to connect!")
#             return
#         print("Connected!")

#         # Wait briefly to allow service discovery to complete
#         await asyncio.sleep(2)
#         await client.get_services()

#         # List all services and characteristics for debugging
#         # print("\n--- Available Services and Characteristics ---")
#         # for service in client.services:
#         #     print(f"Service: {service.uuid}")
#         #     for char in service.characteristics:
#         #         print(f"  Characteristic: {char.uuid}")

#         # Locate the target characteristic
#         target_char = None
#         for service in client.services:
#             if service.uuid.lower() == SERVICE_UUID.lower():
#                 for char in service.characteristics:
#                     if char.uuid.lower() == CHARACTERISTIC_UUID.lower():
#                         target_char = char
#                         break
#                 if target_char is not None:
#                     break

#         if target_char is None:
#             print("Target characteristic not found! Check UUIDs or service discovery.")
#             return
#         else:
#             print("Target characteristic found!")

#         # Read the initial value
#         try:
#             value = await client.read_gatt_char(CHARACTERISTIC_UUID)
#             try:
#                 decoded_value = value.decode('utf-8')
#             except UnicodeDecodeError:
#                 decoded_value = value.hex()
#             print("Initial value:", decoded_value)
#         except Exception as e:
#             print("Error reading characteristic:", e)

#         # Start notifications to receive data updates
#         try:
#             await client.start_notify(CHARACTERISTIC_UUID, notification_handler)
#             print("Started notifications. Listening for updates...")
#         except Exception as e:
#             print("Error starting notifications:", e)

#         # Loop for writing data from user input
#         while True:
#             user_input = await get_user_input("Enter data to write (or 'exit' to quit): ")
#             if user_input.lower() == 'exit':
#                 break
#             try:
#                 await client.write_gatt_char(CHARACTERISTIC_UUID, user_input.encode('utf-8'))
#                 print("Wrote data:", user_input)
#             except Exception as e:
#                 print("Error writing characteristic:", e)

#         # Stop notifications before disconnecting
#         await client.stop_notify(CHARACTERISTIC_UUID)
#         print("Stopped notifications.")

# if __name__ == "__main__":
#     asyncio.run(run())

import asyncio
import time
import datetime
from bleak import BleakClient, BleakScanner

SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"

def notification_handler(sender, data):
    """Callback for handling incoming notifications."""
    try:
        decoded = data.decode('utf-8')
    except UnicodeDecodeError:
        decoded = data.hex()
    
    # Get the current timestamp
    timestamp = time.time()
    
    # Convert to formatted time: HH:MM:SS:MMM
    formatted_time = datetime.datetime.fromtimestamp(timestamp).strftime('%H:%M:%S:%f')[:-3]
    
    # Print the notification with formatted time
    print(f"\n[{formatted_time}] Notification from {sender}: {decoded}")

async def get_user_input(prompt: str) -> str:
    """Asynchronously get user input without blocking the event loop."""
    return await asyncio.to_thread(input, prompt)

async def run():
    print("Scanning for BLE devices...")
    devices = await BleakScanner.discover()
    target = None

    for d in devices:
        print(f"Found device: {d.name}, {d.address}")
        if d.name and "SkateABS" in d.name:
            target = d
            break

    if target is None:
        print("Target device not found. Ensure it is advertising!")
        return

    print(f"Connecting to {target.address}...")
    async with BleakClient(target.address) as client:
        if not client.is_connected:
            print("Failed to connect!")
            return
        print("Connected!")

        await asyncio.sleep(2)
        await client.get_services()

        target_char = None
        for service in client.services:
            if service.uuid.lower() == SERVICE_UUID.lower():
                for char in service.characteristics:
                    if char.uuid.lower() == CHARACTERISTIC_UUID.lower():
                        target_char = char
                        break
                if target_char is not None:
                    break

        if target_char is None:
            print("Target characteristic not found! Check UUIDs or service discovery.")
            return
        else:
            print("Target characteristic found!")

        try:
            value = await client.read_gatt_char(CHARACTERISTIC_UUID)
            try:
                decoded_value = value.decode('utf-8')
            except UnicodeDecodeError:
                decoded_value = value.hex()
            print("Initial value:", decoded_value)
        except Exception as e:
            print("Error reading characteristic:", e)

        try:
            await client.start_notify(CHARACTERISTIC_UUID, notification_handler)
            print("Started notifications. Listening for updates...")
        except Exception as e:
            print("Error starting notifications:", e)

        while True:
            user_input = await get_user_input("Enter data to write (or 'exit' to quit): ")
            if user_input.lower() == 'exit':
                break
            try:
                await client.write_gatt_char(CHARACTERISTIC_UUID, user_input.encode('utf-8'))
                print("Wrote data:", user_input)
            except Exception as e:
                print("Error writing characteristic:", e)

        await client.stop_notify(CHARACTERISTIC_UUID)
        print("Stopped notifications.")

if __name__ == "__main__":
    asyncio.run(run())

