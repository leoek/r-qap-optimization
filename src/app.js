import RQAPParser from "./rqapParser";

import bindings from "bindings";

const addon = bindings("nativeaddon");

console.log("test");

addon.test()

const parser = new RQAPParser();

const main = async () => {
  const parsed = await parser.parseFile();
  console.log(parsed);
}

main()