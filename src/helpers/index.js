export const objectValues = obj => Object.keys(obj).map((key) => {
  return obj[key];
});

export const sleep = (ms) => new Promise(resolve=>{
      setTimeout(resolve, ms)
  })


export const newMatrix = (x, y, initVal = 0) => [...Array(x)].map(_ => [...Array(y)].map(_ => initVal));