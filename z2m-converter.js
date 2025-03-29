const { Zcl } = require('zigbee-herdsman');
const exposes = require('zigbee-herdsman-converters/lib/exposes');
const mExt = require('zigbee-herdsman-converters/lib/modernExtend');
const fz = require('zigbee-herdsman-converters/converters/fromZigbee');
const tz = require('zigbee-herdsman-converters/converters/toZigbee');
const { getFromLookup, assertString } = require('zigbee-herdsman-converters/lib/utils');
const reporting = require('zigbee-herdsman-converters/lib/reporting');
const e = exposes.presets;
const ea = exposes.access;

const manufacturerCode = 0x1037;
const operationModes = ["not_set", "toggle", "momentary", "multistate"];
const modeTZLookup = {};
const modeFZLookup = {};
operationModes.forEach((mode, index) => {
    modeTZLookup[mode] = index;
    modeFZLookup[index] = mode;
});

const operation_mode = {
    fz: {
        cluster: "ManuConfiguration",
        type: ["attributeReport", "readResponse"],
        convert: (model, msg, publish, options, meta) => {
            const data = msg.data;
            if (data.hasOwnProperty("buttonMode")) {
                return { operation_mode: getFromLookup(data.buttonMode, modeFZLookup) };
            }
        }
    },
    tz: {
        key: ["operation_mode"],
        convertSet: async (entity, key, value, meta) => {
            assertString(value);
            const endpoint = meta.device.getEndpoint(1);
            await endpoint.write(
                "ManuConfiguration",
                { buttonMode: getFromLookup(value.toLowerCase(), modeTZLookup) },
                { manufacturerCode: manufacturerCode },
            );
            return { state: { operation_mode: value.toLowerCase() } };
        },
        convertGet: async (entity, key, meta) => {
            const endpoint = meta.device.getEndpoint(1);
            await endpoint.read(
                "ManuConfiguration",
                ["buttonMode"],
                { manufacturerCode: manufacturerCode },
            );
        },
    }
}

const definition = [
    {
        zigbeeModel: ["lumi.remote.b286acn02.alt"],
        model: "WXKG07LM",
        vendor: "Aqara",
        description: "Wireless remote switch D1 (double rocker)",
        fromZigbee: [fz.battery, operation_mode.fz],
        toZigbee: [operation_mode.tz],
        // endpoint: (device) => {
        //  return {left: 1, right: 2, both: 3};
        // },
        exposes: [
            e.battery(),
            e.battery_voltage(),
            e.enum("operation_mode", ea.ALL, operationModes),
        ],
        extend: [
            mExt.deviceAddCustomCluster("ManuConfiguration", {
                ID: 0xFC00,
                manufacturerCode: manufacturerCode,
                attributes: {
                    buttonMode: { ID: 0x0000, type: Zcl.DataType.ENUM8 }
                },
                commands: {},
                commandsResponse: {},
            })
        ],
        configure: async (device, coordinatorEndpoint) => {
            const endpoint = device.getEndpoint(1);
            await reporting.bind(endpoint, coordinatorEndpoint, ["genPowerCfg", "ManuConfiguration"]);
        },
    }
]

module.exports = definition;
