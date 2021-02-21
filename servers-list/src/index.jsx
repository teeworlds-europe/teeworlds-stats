import "core-js/stable";
import "regenerator-runtime/runtime";
import React, { useState, useEffect } from 'react';
import ReactDOM from 'react-dom';
import axios from 'axios';
import App from './components/App.jsx';

const PlayersList = () => {
  return (
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
  );
};

const ServerItem = props => {
  const [isExpanded, setIsExpanded] = useState(false);

  let server = props.server;
  if (server.state == 'unreachable') {
    return (
      <tr>
        <th scope="row">{server.name}</th>
        <td>-</td>
        <td>-</td>
        <td>-/-</td>
      </tr>
    );
  }
  return (
    <tr>
      <th scope="row">{server.name}</th>
      <td>{server.gametype}</td>
      <td>{server.map}</td>
      <td>{server.num_clients}/{server.max_clients}</td>
    </tr>
  );
}

ReactDOM.render(<App />, document.getElementById("root"));
