import axios from 'axios';

const searchServers = async (endpoint='/servers.json') => {
  let fetchedServers = [];
  (await axios.get('/servers.json'))
    .data.split('\n')
    .filter(line => line.length > 0)
    .forEach(line => {
      fetchedServers.push((JSON.parse(line)));
    });
  return fetchedServers;
};

export {
  searchServers
};
