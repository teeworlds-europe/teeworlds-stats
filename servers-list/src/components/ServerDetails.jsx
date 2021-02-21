import React, { useState, useEffect } from 'react';

const ServerDetails = (props) => {
  let renderServer = null;
  for (const server of props.servers) {
    if (
      server.address == props.match.params.server
      && server.port == props.match.params.port
    ) {
      renderServer = server;
      break;
    }
  }
  let content = '';
  let title;
  if (props.servers.length === 0) {
    title = <h1>Loading server status...</h1>;
  } else if (renderServer === null) {
    title = <h1>Server has been not found</h1>;
  } else {
    const players = renderServer.clients.map((player) => {
      return (
        <tr key={
          player.name
          + player.clan
          + player.country.toString()
          + player.score.toString()
          + player.team.toString()
        }>
          <th scope="row">{player.name}</th>
          <td>{player.clan}</td>
          <td>{player.score}</td>
        </tr>
      );
    });
    title = (
      <header>
        <dl>
          <dt>Server name</dt>
          <dd>{renderServer.name}</dd>
          <dt>Address</dt>
          <dd>{renderServer.address}</dd>
          <dt>Port</dt>
          <dd>{renderServer.port}</dd>
          <dt>Game mode</dt>
          <dd>{renderServer.gametype}</dd>
          <dt>Map</dt>
          <dd>{renderServer.map}</dd>
          <dt>Players</dt>
          <dd>{renderServer.num_clients}/{renderServer.max_clients}</dd>
          <dt>Status</dt>
          <dd>{renderServer.state}</dd>
          <dt>Last status update</dt>
          <dd>{new Date(Number(renderServer.timestamp) * 1000).toLocaleString('pl')}</dd>
        </dl>
      </header>
    );
    content = (
      <table>
        <thead>
          <tr>
            <th scope="col">Player</th>
            <th scope="col">Clan</th>
            <th scope="col">Score</th>
          </tr>
        </thead>
        <tbody>{players}</tbody>
      </table>
    );
  }
  return (
    <section className="server-details">
      {title}
      {content}
    </section>
  );
};

export default ServerDetails;
