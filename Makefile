# SDCC 編譯：make 產生 build/led_os.hex
CC = sdcc
BUILD = build

all: $(BUILD)/led_os.hex

$(BUILD)/%.rel: %.c uos.h | $(BUILD)
	$(CC) -c $< -o $@

$(BUILD)/led_os.ihx: $(BUILD)/main.rel $(BUILD)/uos.rel
	$(CC) $^ -o $@

$(BUILD)/led_os.hex: $(BUILD)/led_os.ihx
	packihx $< > $@

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)

.PHONY: all clean
