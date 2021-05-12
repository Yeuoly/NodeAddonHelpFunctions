const hello = require('../build/Release/yfunction');
const Socket = hello.NodeAddonSocket;

const start = (...args) => new Promise(async resolve => {
    const sock = new Socket(...args);

});

(async function(){
    await start("127.0.0.1", 10010, i => {
        console.log(i);
    }, str => {
        console.log(str);
    });
})();