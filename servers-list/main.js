import { h, Component, render } from 'https://unpkg.com/preact@latest?module';
import { useState, useEffect } from 'https://unpkg.com/preact@latest/hooks/dist/hooks.module.js?module';
import htm from 'https://unpkg.com/htm@latest?module';

// Initialize htm with Preact
const html = htm.bind(h);

//function onListRefresh() {
  //console.log(this.responseText);
  //let r = [];
  //try {
    //this.responseText.split('\n')
    //.filter(line => line.length > 0)
    //.forEach(line => {
      //console.log(JSON.parse(line));
    //});
  //} catch {
    //console.error('Failed to process fetched server status');
  //}
//}

//let req = new XMLHttpRequest();
//req.addEventListener('load', onListRefresh);
//req.open('GET', '/servers.json');
//req.send();

const PlayersList = () => {
  return html`
    <table>
      <thead>
        <tr>
          <th scope="col">Player</th>
          <th scope="col">Clan</th>
          <th scope="col">Score</th>
        </tr>
      </thead>
      <tbody>
        <th scope="row">Example player</th>
        <td>Example clan</td>
        <td>Example score</td>
      </tbody>
    </table>
  `;
};

const ServerItem = props => {
  const [isExpanded, setIsExpanded] = useState(false);

  let server = props.server;
  if (server.state == 'unreachable') {
    return html`
      <tr>
        <th scope="row">${server.name}</th>
        <td>-</td>
        <td>-</td>
        <td>-/-</td>
      </tr>
    `;
  }
  return html`
    <tr>
      <th scope="row">${server.name}</th>
      <td>${server.gametype}</td>
      <td>${server.map}</td>
      <td>${server.num_clients}/${server.max_clients}</td>
    </tr>
  `;
}

const App = () => {
  const [serverName, setServerName] = useState('Europe');
  const [playerName, setPlayerName] = useState('');
  const [servers, setServers] = useState([]);

  const onServerInput = e => {
    setServerName(e.target.value);
  };

  const onPlayerInput = e => {
    setPlayerName(e.target.value);
  };

  const onSearchSubmit = e => {
    e.preventDefault();
    alert(`Submitted ${serverName}`);
  };

  const searchServers = () => {
    let xhr = new XMLHttpRequest();
    xhr.onload = () => {
      try {
        let fetchedServers = [];
        xhr.response.split('\n')
        .filter(line => line.length > 0)
        .forEach(line => {
          fetchedServers.push((JSON.parse(line)));
        });
        setServers(fetchedServers);
      } catch {
        console.error('Failed to process fetched server status');
      }
    }
    xhr.open('GET', '/servers.json');
    xhr.send();
  }

  useEffect(() => {
    searchServers();
  }, []);

  let serversList = [...servers]
  .sort((a, b) => {
    const score = server => {
      let score = 0;
      if (server.state == 'unreachable') {
        return 0;
      }
      if (server.name.toLowerCase().includes(serverName) && serverName != '') {
        score += 1;
      }
      for (const client of server.clients) {
        if (client.name.toLowerCase().includes(playerName) && playerName != '') {
          score += 1;
        }
      }
      return score;
    };
    let scoreA = score(a);
    let scoreB = score(b);
    if (scoreA == scoreB) {
      return 0;
    } else if (scoreA > scoreB) {
      return -1;
    } else {
      return 1;
    }
  })
  .map(server => {
    return html`<${ServerItem} server=${server}/>`;
  });

  return html`
    <form id="server-search" onSubmit=${onSearchSubmit}>
      <label for="server-name">Server: </label>
      <input type="text" id="server-name" value=${serverName} onInput=${onServerInput} />
      <label for="player-name">Player: </label>
      <input type="text" id="player-name" value=${playerName} onInput=${onPlayerInput} />
      <button type="submit">Search</button>
    </form>
    <table>
      <thead>
        <tr>
          <th scope="col">Server</th>
          <th scope="col">Type</th>
          <th scope="col">Map</th>
          <th scope="col">Players</th>
        </tr>
      </thead>
      <tbody>${serversList}</tbody>
    </table>
  `;
};

render(html`<${App} />`, document.body);
