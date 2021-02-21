import React, { useState, useEffect } from 'react';
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';
import {
  faCheckCircle,
  faTimesCircle,
} from '@fortawesome/free-solid-svg-icons';

const ServersList = (props) => {
  const switchRoute = (location) => {
    props.history.push(location);
  };

  const serversList = props.servers
    .map(server => {
      const key = `${server.address}/${server.port.toString()}`
      const {
        name,
        state,
        gametype = '-',
        map = '-',
        num_clients = '-',
        max_clients = '-'
      } = server;
      const stateIcon = state == 'online'
        ? <FontAwesomeIcon icon={faCheckCircle} />
        : <FontAwesomeIcon icon={faTimesCircle} />;
      return (
        <tr key={key} onClick={() => switchRoute('/servers/' + key)}>
          <th scope="row">{name}</th>
          <td>{stateIcon}</td>
          <td>{gametype}</td>
          <td>{map}</td>
          <td>{num_clients}/{max_clients}</td>
        </tr>
      );
    });

  return (
    <section className="server-list">
      <table>
        <thead>
          <tr>
            <th scope="col">Server</th>
            <th scope="col">Online</th>
            <th scope="col">Game mode</th>
            <th scope="col">Map</th>
            <th scope="col">Players</th>
          </tr>
        </thead>
        <tbody>{serversList}</tbody>
      </table>
    </section>
  );
};

export default ServersList;
