import asyncio
import websockets
import keyboard
import time  # For tracking debounce time

DEBOUNCE_INTERVAL = 1.2  # Set debounce time in seconds (200 ms)
MAX_SPEED = 255  # Set the maximum speed of the motors


async def communicate():
    uri = "ws://192.168.4.1:81"  # Replace with the ESP32's IP
    last_sent_time = 0
    last_combination = ""

    # Dictionary to track the state of each arrow key (pressed or not)
    key_states = {"up": False, "down": False, "left": False, "right": False}

    try:
        async with websockets.connect(uri) as websocket:
            print("Ready to receive arrow key commands...")

            while True:
                current_time = time.time()

                # Update key_states based on current key presses (non-blocking)
                key_states["up"] = keyboard.is_pressed("up")
                key_states["down"] = keyboard.is_pressed("down")
                key_states["left"] = keyboard.is_pressed("left")
                key_states["right"] = keyboard.is_pressed("right")

                # Calculate motor weights based on current key states
                LEFT_MOTOR_WEIGHT = 0
                RIGHT_MOTOR_WEIGHT = 0
                new_combination = ""

                # Process key presses and handle combinations
                if key_states["up"]:
                    LEFT_MOTOR_WEIGHT += MAX_SPEED
                    RIGHT_MOTOR_WEIGHT += MAX_SPEED
                    new_combination += "up"

                if key_states["down"]:
                    LEFT_MOTOR_WEIGHT -= MAX_SPEED
                    RIGHT_MOTOR_WEIGHT -= MAX_SPEED
                    new_combination += "down"

                if key_states["left"]:
                    LEFT_MOTOR_WEIGHT -= MAX_SPEED if LEFT_MOTOR_WEIGHT > -MAX_SPEED else 0
                    RIGHT_MOTOR_WEIGHT += MAX_SPEED if RIGHT_MOTOR_WEIGHT < MAX_SPEED else 0
                    new_combination += "left"

                if key_states["right"]:
                    LEFT_MOTOR_WEIGHT += MAX_SPEED if LEFT_MOTOR_WEIGHT < MAX_SPEED else 0
                    RIGHT_MOTOR_WEIGHT -= MAX_SPEED if RIGHT_MOTOR_WEIGHT > -MAX_SPEED else 0
                    new_combination += "right"

                # Handle simultaneous 'up' and 'down' presses (set weights to 0)
                if key_states["up"] and key_states["down"]:
                    LEFT_MOTOR_WEIGHT = 0
                    RIGHT_MOTOR_WEIGHT = 0
                    new_combination = "up+down"

                # If no keys are pressed, send 0, 0
                if not any(key_states.values()):
                    LEFT_MOTOR_WEIGHT = 0
                    RIGHT_MOTOR_WEIGHT = 0
                    new_combination = "stop"

                # Debounce and send motor weights only if they have changed
                if (
                    current_time - last_sent_time > DEBOUNCE_INTERVAL
                    or new_combination != last_combination
                ):
                    await websocket.send(f"{LEFT_MOTOR_WEIGHT},{RIGHT_MOTOR_WEIGHT}")
                    last_sent_time = current_time
                    last_combination = new_combination
                    print(f"Sent: {LEFT_MOTOR_WEIGHT} {RIGHT_MOTOR_WEIGHT}")

                await asyncio.sleep(0.01)  # Small sleep to prevent CPU overload

    except asyncio.TimeoutError:
        print("Connection timed out. Check the server IP and try again.")
    except websockets.exceptions.ConnectionClosedError:
        print("Connection closed unexpectedly. Ensure the server is reachable.")


# Run the client
asyncio.run(communicate())