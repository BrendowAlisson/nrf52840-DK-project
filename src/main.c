#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/util.h>
#include <sys/printk.h>
#include <inttypes.h>
#include <drivers/i2c.h>

#define SW0_NODE DT_ALIAS(sw0)
#define SW1_NODE DT_ALIAS(sw1)
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define TWI DT_LABEL(DT_NODELABEL(i2c0))
#define GPIO1_PORT DT_LABEL(DT_NODELABEL(gpio1))
#define I2C_SLV_ADDR 0x32
#define SLEEP_TIME_US 500

static const struct gpio_dt_spec button_0 = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
static const struct gpio_dt_spec button_1 = GPIO_DT_SPEC_GET(SW1_NODE, gpios);
static struct gpio_dt_spec led_0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static struct gpio_dt_spec led_1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
int sequence_button_pressed[2] = {0, 1};

void blink(bool flag)
{
	gpio_pin_set_dt(&led_0, (int)flag);
	k_usleep(SLEEP_TIME_US);
}

bool deviceNotExists(const struct device *dvc)
{	
	if (dvc == NULL)
	{
		printk("I2C: Device driver not found\n");
		return true;
	}

	return false;
}

bool isPressed(int state_button) 
{
	if(state_button == 0){
		return true;
	}

	return false;
}

void updateArrayPositions(int *sequence_button_pressed, int first_position)
{
	if(first_position == sequence_button_pressed[0])
	{
		return;
	}

	int aux = sequence_button_pressed[0];
	sequence_button_pressed[0] = sequence_button_pressed[1];
	sequence_button_pressed[1] = aux;
}

void readSensor(const struct device *I2C, uint8_t data)
{
	i2c_reg_read_byte(I2C, I2C_SLV_ADDR, 0x0F, &data);
	printk("%u", data);
}

void toggleLED(int state)
{
	gpio_pin_set_dt(&led_1, state);
}

int resetCounter(const struct device *RESET, int button_toggle, int toggle_flag)
{
	if (button_toggle != toggle_flag)
	{
		gpio_pin_set(RESET, 11, 0);
		toggle_flag = button_toggle;
		return toggle_flag;
	}
	
	gpio_pin_set(RESET, 11, 1);
	return toggle_flag;
}

void orderButtonPressSequence(int button_0_state, int button_1_state, int *sequence_button_pressed)
{
	if (isPressed(button_0_state) && !isPressed(button_1_state))
	{
		updateArrayPositions(sequence_button_pressed, 0);
	}
	else if (!isPressed(button_0_state) && isPressed(button_1_state))
	{
		 updateArrayPositions(sequence_button_pressed, 1);
	}
}

bool checkButtonOrder(int first_position, int button)
{
	if(button == first_position)
	{
		return true;
	}
	
	return false;
}

int main()
{
	const struct device *I2C = device_get_binding(TWI);
	const struct device *LOOP = device_get_binding(GPIO1_PORT);
	const struct device *RESET = device_get_binding(GPIO1_PORT);

	gpio_pin_configure_dt(&button_0, GPIO_INPUT | GPIO_PULL_UP);
	gpio_pin_configure_dt(&button_1, GPIO_INPUT | GPIO_PULL_UP);
	gpio_pin_configure_dt(&led_0, GPIO_OUTPUT_LOW);
	gpio_pin_configure_dt(&led_1, GPIO_OUTPUT_LOW);
	gpio_pin_configure(LOOP, 10, GPIO_INPUT | GPIO_PULL_UP);
	gpio_pin_configure(RESET, 11, GPIO_OUTPUT_HIGH);

	int button_toggle = gpio_pin_get(LOOP, 10);
	int toggle_flag = button_toggle;
	bool led_blink_status = true;
	uint8_t data = 1;

	if (deviceNotExists(I2C))
	{
		return 0;
	}

	printk("I2C: Device driver found!!!\n");
	i2c_configure(I2C, I2C_SPEED_SET(I2C_SPEED_ULTRA));

	while (true)
	{
		int button_0_state = gpio_pin_get_dt(&button_0);
		int button_1_state = gpio_pin_get_dt(&button_1);
		button_toggle = gpio_pin_get(LOOP, 10);

		orderButtonPressSequence(button_0_state, button_1_state, sequence_button_pressed);
		toggle_flag = resetCounter(RESET, button_toggle, toggle_flag);

		if (isPressed(button_0_state))
		{
			blink(led_blink_status);
			led_blink_status = !led_blink_status;

			if (checkButtonOrder(sequence_button_pressed[0], 0))
			{
				readSensor(I2C, data);
			}
		}

		if (isPressed(button_1_state) && checkButtonOrder(sequence_button_pressed[0], 1))
		{
			toggleLED(1);
		}
		else
		{
			toggleLED(0);
		}

	}
}