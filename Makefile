default:
	@echo "run \"make TARGET\" where TARGET could be one of:"
	@echo "  test"
	@echo "  dist"
	@echo "  clean"

dist:
	@mkdir -p tmp
	@zip -r tmp/esensors-websensor.zip esensors-websensor
	@echo built tmp/esensors-websensor.zip

test:
	prove -v t/*.t

clean:
	rm -rf tmp/

.PHONY: default dist test clean
